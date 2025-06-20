// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionInventoryEdNode_ItemCategory.h"
#include "Inventory/OrionInventoryNode.h"
#include "Inventory/OrionInventorySetting.h"
#include "DialogBuilderNode.h"

FText UOrionInventoryEdNode_ItemCategory::GetNodeTitle(ENodeTitleType::Type TitleType) const
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

FLinearColor UOrionInventoryEdNode_ItemCategory::GetBackgroundColor() const
{
	return GetDefault<UOrionInventorySetting>()->ItemCategoryColor;
}

FText UOrionInventoryEdNode_ItemCategory::GetTooltipText() const
{
	return Super::GetTooltipText();
}

void UOrionInventoryEdNode_ItemCategory::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
}
