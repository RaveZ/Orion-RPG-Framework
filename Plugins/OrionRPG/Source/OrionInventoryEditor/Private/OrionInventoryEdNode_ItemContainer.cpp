// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionInventoryEdNode_ItemContainer.h"
#include "Inventory/OrionInventoryNode.h"
#include "Inventory/OrionInventorySetting.h"
#include "DialogBuilderNode.h"

FText UOrionInventoryEdNode_ItemContainer::GetNodeTitle(ENodeTitleType::Type TitleType) const
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

FLinearColor UOrionInventoryEdNode_ItemContainer::GetBackgroundColor() const
{
	return GetDefault<UOrionInventorySetting>()->ItemContainerColor;
}

FText UOrionInventoryEdNode_ItemContainer::GetTooltipText() const
{
	return Super::GetTooltipText();
}

void UOrionInventoryEdNode_ItemContainer::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
}
