// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionInventoryNode_ItemDefinition.h"
#include "Inventory/OrionInventoryItemFragment.h"
#include "Inventory/OrionInventoryGraph.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"


#define LOCTEXT_NAMESPACE "OrionInventoryNode_ItemDefinition"



UOrionInventoryNode_ItemDefinition::UOrionInventoryNode_ItemDefinition()
{
    // Example: Load a texture asset from plugin resources
    auto ItemIconObj = ConstructorHelpers::FObjectFinder<UTexture2D>(TEXT("Texture2D'/OrionRPG/Widgets/Assets/InventoryItem.InventoryItem'"));
    if (ItemIconObj.Succeeded())
    {
        Icon = ItemIconObj.Object;
    }
	MaxStackCount = 1;

}

void UOrionInventoryNode_ItemDefinition::LoadPrerequisites()
{
	TObjectPtr<UOrionInventoryFragment_DropItem> InNewDropItemFragmentTemplate = DropItemFragmentOverride ? DropItemFragmentOverride : GetOwningOrionInventoryGraph()->DefaultDropItemFragment;
	DropItemFragment = InNewDropItemFragmentTemplate && bCanDropItem ? Cast<UOrionInventoryFragment_DropItem>(StaticDuplicateObject(InNewDropItemFragmentTemplate, this, NAME_None, RF_Transactional)) : nullptr;
}

const UOrionInventoryItemFragment* UOrionInventoryNode_ItemDefinition::FindFragmentByClass(TSubclassOf<UOrionInventoryItemFragment> FragmentClass) const
{
	if (FragmentClass != nullptr)
	{
		for (UOrionInventoryItemFragment* Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}

	return nullptr;
}

UOrionInventoryFragment_DropItem* UOrionInventoryNode_ItemDefinition::GetDropItemFragment()
{
	return DropItemFragment;
}

#if WITH_EDITOR
FText UOrionInventoryNode_ItemDefinition::GetNodeTitle() const
{
    return DisplayName;
}

void UOrionInventoryNode_ItemDefinition::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}

FText UOrionInventoryNode_ItemDefinition::GetNodeDescription() const
{
	return FText();
}
#endif


#undef LOCTEXT_NAMESPACE

