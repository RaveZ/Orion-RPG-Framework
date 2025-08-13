// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionInventoryNode_Item.h"


#define LOCTEXT_NAMESPACE "InventorySystemGraphNode"



UOrionInventoryNode_Item::UOrionInventoryNode_Item()
{
}

#if WITH_EDITOR
FText UOrionInventoryNode_Item::GetNodeTitle() const
{
    return DisplayName.ToString().Len() ? DisplayName : FText::FromString("Item");
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

