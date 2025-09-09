// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionInventoryNode_ItemContainer.h"


#define LOCTEXT_NAMESPACE "InventorySystemGraphNode"


UOrionInventoryNode_ItemContainer::UOrionInventoryNode_ItemContainer()
{
    GridCountX = 7;
}

#if WITH_EDITOR


FText UOrionInventoryNode_ItemContainer::GetNodeTitle() const
{
    return Category;
}

void UOrionInventoryNode_ItemContainer::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}

FText UOrionInventoryNode_ItemContainer::GetNodeDescription() const
{
	return FText();
}
#endif


#undef LOCTEXT_NAMESPACE

