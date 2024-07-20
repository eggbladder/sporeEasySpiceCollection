// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
using namespace ArgScript;
using namespace Simulator;



int hasitem(eastl::vector<eastl::intrusive_ptr<Simulator::cSpaceInventoryItem>> inventory, uint32_t id) {
	int i = 0;
	for (auto item : inventory) {
		if (item->mItemID.instanceID == id) {
			return i;
		} 
		i += 1;
	}
	return -1;
}
void subtractspice(cPlanetRecord* planet, int amount) {
	int amountleft = amount;
	auto listofcities = planet->mCivData[0]->mCities;
	for (auto city : listofcities) {
		auto cityspice = city->mSpiceProduction;
		if (cityspice >= amountleft) {
			city->mSpiceProduction = cityspice - amountleft;
			return;
		} 
		else {
			city->mSpiceProduction = 0;
			amountleft -= cityspice;
		}
	}
}

void Initialize()
{
	// This method is executed when the game starts, before the user interface is shown
	// Here you can do things such as:
	//  - Add new cheats
	//  - Add new simulator classes
	//  - Add new game modes
	//  - Add new space tools
	//  - Change materials


}
//member_detour(CreateSpaceCommEvent_detour, Simulator::cComm auto (uint32_t, PlanetID, uint32_t, uint32_t, void*, int, unisgned int)) {


//};
member_detour(GetEmpireForStar__detour, Simulator::cStarManager, cEmpire* (cStarRecord*)) {
	cEmpire* detoured(cStarRecord * starRecord) {
		auto empire = original_function(this, starRecord);
		if (empire && empire == GetPlayerEmpire() && SimulatorSpaceGame.Get()) {
			for (auto planet : starRecord->GetPlanetRecords()) {
				if (planet && planet->GetTechLevel() == TechLevel::Empire) {
					auto cSpaceTrading = cSpaceTrading::Get();
					auto playerinv = SimulatorSpaceGame.GetPlayerInventory();					auto spiceamount = cPlanetRecord::CalculateSpiceProduction(planet.get(), 0);
					auto spicetype = planet->mSpiceGen;
					auto playerinventory = playerinv->mInventoryItems; 
					auto index = hasitem(playerinventory, spicetype.instanceID);
					auto number = playerinv->mMaxItemCountPerItem; 
					bool add = false;
					if (index != -1) {
						auto count = playerinventory[index]->mItemCount;
						if (count + spiceamount > number) {
							spiceamount = number - count;
						}
						add = true;
					}
					else if (playerinv->GetAvailableCargoSlotsCount() != 0) {
						add = true;
					}
					if (add && spiceamount >= 1) {
						cSpaceTrading->ObtainTradingObject(spicetype, spiceamount);
						subtractspice(planet.get(), spiceamount);
					}
					
					
			
				}
				
			}
		}

		return empire;
	}

};

void Dispose()
{
	// This method is called when the game is closing
}
void AttachDetours() {
	// Call the attach() method on any detours you want to add
	// For example: cViewer_SetRenderType_detour::attach(GetAddress(cViewer, SetRenderType));
	GetEmpireForStar__detour::attach(GetAddress(cStarManager, GetEmpireForStar));
}


// Generally, you don't need to touch any code here
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		ModAPI::AddPostInitFunction(Initialize);
		ModAPI::AddDisposeFunction(Dispose);

		PrepareDetours(hModule);
		AttachDetours();
		CommitDetours();
		break;

	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}

