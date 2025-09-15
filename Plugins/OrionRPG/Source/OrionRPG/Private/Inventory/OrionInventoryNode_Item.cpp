// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionInventoryNode_Item.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"


#define LOCTEXT_NAMESPACE "InventorySystemGraphNode"



UOrionInventoryNode_Item::UOrionInventoryNode_Item()
{
    // Example: Load a texture asset from plugin resources
    auto ItemIconObj = ConstructorHelpers::FObjectFinder<UTexture2D>(TEXT("Texture2D'/OrionRPG/Widgets/Assets/InventoryItem.InventoryItem'"));
    if (ItemIconObj.Succeeded())
    {
        ItemDefinition.Icon = ItemIconObj.Object;
    }
}

#if WITH_EDITOR
FText UOrionInventoryNode_Item::GetNodeTitle() const
{
    return ItemDefinition.DisplayName;
}

void UOrionInventoryNode_Item::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}

FText UOrionInventoryNode_Item::GetNodeDescription() const
{
	return FText();
}
#endif


#undef LOCTEXT_NAMESPACE

