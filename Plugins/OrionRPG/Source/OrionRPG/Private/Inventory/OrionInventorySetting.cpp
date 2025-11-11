// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionInventorySetting.h"
#include "UObject/ConstructorHelpers.h"

UOrionInventorySetting::UOrionInventorySetting()
{
	auto ItemViewerFinder = ConstructorHelpers::FClassFinder<AActor>(TEXT("/OrionRPG/Blueprint/Inventory/BP_InventoryItemViewer.BP_InventoryItemViewer_C"));
	if (ItemViewerFinder.Succeeded())
	{
		ItemViewerActorClass = ItemViewerFinder.Class;
	}

	auto CharacterDisplayFinder = ConstructorHelpers::FClassFinder<AActor>(TEXT("/OrionRPG/Blueprint/Inventory/BP_InventoryCharacterDisplay.BP_InventoryCharacterDisplay_C"));
	if (CharacterDisplayFinder.Succeeded())
	{
		CharacterDisplayActorClass = CharacterDisplayFinder.Class;
	}

	ItemViewerLocation = FVector(0.f, 0.f, 1000000.f);
	CharacterDisplayLocation = FVector(1000.f, 0.f, 1000000.f);

	ItemContainerColor = FLinearColor(0.07f, 0.07f, 0.07f);
	bCanRenameNode = true;
}


UOrionInventorySetting::~UOrionInventorySetting()
{
}
