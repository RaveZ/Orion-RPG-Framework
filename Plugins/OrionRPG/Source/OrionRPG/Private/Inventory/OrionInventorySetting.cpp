// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionInventorySetting.h"
#include "Inventory/OrionActorViewer.h"
#include "Inventory/OrionItemPickup.h"
#include "UI/Foundation/OrionGridBase.h"
#include "UObject/ConstructorHelpers.h"

UOrionInventorySetting::UOrionInventorySetting()
{

	auto ItemViewerFinder = ConstructorHelpers::FClassFinder<AOrionActorViewer>(TEXT("/OrionRPG/Blueprint/Inventory/BP_ItemViewer.BP_ItemViewer_C"));
	if (ItemViewerFinder.Succeeded())
	{
		ItemViewerClass = ItemViewerFinder.Class;
	}

	auto CharacterViewerFinder = ConstructorHelpers::FClassFinder<AOrionActorViewer>(TEXT("/OrionRPG/Blueprint/Inventory/BP_CharacterViewer.BP_CharacterViewer_C"));
	if (CharacterViewerFinder.Succeeded())
	{
		CharacterViewerClass = CharacterViewerFinder.Class;
	}


	auto PickupActorFinder = ConstructorHelpers::FClassFinder<AOrionItemPickup>(TEXT("/OrionRPG/Blueprint/Inventory/BP_PickupItem.BP_PickupItem_C"));
	if (PickupActorFinder.Succeeded())
	{
		DefaultPickupActorClass = PickupActorFinder.Class;
	} 
	auto LootableActorFinder = ConstructorHelpers::FClassFinder<AOrionLootable>(TEXT("/OrionRPG/Blueprint/Inventory/BP_LootableItem.BP_LootableItem_C"));
	if (LootableActorFinder.Succeeded())
	{
		DefaultLootableActorClass = LootableActorFinder.Class;
	}
	

	ItemViewerLocation = FVector(0.f, 0.f, 1000000.f);
	CharacterViewerLocation = FVector(10000.f, 0.f, 1000000.f);

	ItemContainerColor = FLinearColor(0.07f, 0.07f, 0.07f);
	bCanRenameNode = true;
}


UOrionInventorySetting::~UOrionInventorySetting()
{
}
