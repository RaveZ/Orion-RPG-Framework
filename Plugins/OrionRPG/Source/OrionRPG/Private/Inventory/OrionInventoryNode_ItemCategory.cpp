// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionInventoryNode_ItemCategory.h"
#include "Kismet2/Kismet2NameValidators.h"


#define LOCTEXT_NAMESPACE "DialogSystemGraphNode"



#if WITH_EDITOR

UOrionInventoryNode_ItemCategory::UOrionInventoryNode_ItemCategory()
{
}

FText UOrionInventoryNode_ItemCategory::GetNodeTitle() const
{
    return FText::FromString("Item Container");
}

void UOrionInventoryNode_ItemCategory::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}

FText UOrionInventoryNode_ItemCategory::GetNodeDescription() const
{
	return FText();
}
#endif


#undef LOCTEXT_NAMESPACE

