// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionInventoryEdSubNode_Item.h"
#include "Inventory/OrionInventoryNode.h"
#include "Inventory/OrionInventorySetting.h"
#include "DialogBuilderNode.h"
#include "Inventory/OrionInventoryNode_Item.h"

FText UOrionInventoryEdSubNode_Item::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const UOrionInventoryNode* InventoryNode = Cast<UOrionInventoryNode>(NodeInstance);

	if (InventoryNode != NULL)
	{
		return InventoryNode->GetNodeTitle();
	}
	else if (!ClassData.GetClassName().IsEmpty())
	{
		FString StoredClassName = ClassData.GetClassName();
		StoredClassName.RemoveFromEnd(TEXT("_C"));

		return FText::Format(NSLOCTEXT("InventoryGraph", "NodeClassError", "Class {0} not found, make sure it's saved!"), FText::FromString(StoredClassName));
	}

	return Super::GetNodeTitle(TitleType);
}

FLinearColor UOrionInventoryEdSubNode_Item::GetBackgroundColor() const
{
	return GetDefault<UOrionInventorySetting>()->ItemContainerColor;
}

FText UOrionInventoryEdSubNode_Item::GetTooltipText() const
{
	return Super::GetTooltipText();
}

void UOrionInventoryEdSubNode_Item::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
}

const FSlateBrush* UOrionInventoryEdSubNode_Item::GetItemImage() const
{
	return FAppStyle::GetBrush(TEXT("Sequencer.SetKeyAll"));
	
}
