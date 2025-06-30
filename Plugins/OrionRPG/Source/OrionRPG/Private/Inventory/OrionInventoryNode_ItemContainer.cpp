// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionInventoryNode_ItemContainer.h"
#include "Kismet2/Kismet2NameValidators.h"


#define LOCTEXT_NAMESPACE "InventorySystemGraphNode"



#if WITH_EDITOR

UOrionInventoryNode_ItemContainer::UOrionInventoryNode_ItemContainer()
{
    GridCountX = 7;
}

FText UOrionInventoryNode_ItemContainer::GetNodeTitle() const
{
    return Category.ToString().Len() ? Category : FText::FromString("Item Container");
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

