// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "UI/Foundation/OrionGridBase.h"
#include "Inventory/OrionInventoryItemInstance.h"
#include "Inventory/OrionInventoryFunctionLibrary.h"
#include "CommonActionWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OrionGridBase)

//////////////////////////////////////////////////////////////////////

// Orion Grid Base
void UOrionGridBase::NativePreConstruct()
{
	Super::NativePreConstruct();
}

//////////////////////////////////////////////////////////////////////

// Orion Grid Slot
void UOrionGridSlot::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UOrionGridSlot::SetGridItem_Implementation(UOrionGridItem* InGridItem)
{
	if (GridItem == InGridItem) return;

	if (GridItem)
	{
		GridItem->OnItemRemoved.Clear();
		GridItem->OnItemChanged.Clear();
	}

	GridItem = InGridItem;
	
	if (GridItem)
	{
		GridItem->OnItemRemoved.AddDynamic(this, &UOrionGridSlot::HandleGridItemRemoved);
		GridItem->OnItemChanged.AddDynamic(this, &UOrionGridSlot::HandleGridItemChanged);
	}
	//TODO: bind event like on grid remved or changed to check if the item is still valid
	//GridItem->OnGridItemRemoved.AddDynamic(this, &UOrionGridSlot::HandleGridItemRemoved);
	//GridItem->OnGridItemChanged.AddDynamic(this, &UOrionGridSlot::HandleGridItemChanged);

	//if grid item removed->self->GridItem = nullptr;
	//else if grid changed-> also change the item dimension according to item instance size
}

void UOrionGridSlot::HandleGridItemRemoved_Implementation()
{
	SetGridItem(nullptr);
}

void UOrionGridSlot::HandleGridItemChanged_Implementation(UOrionInventoryItemInstance* InItemInstance)
{
}

//////////////////////////////////////////////////////////////////////

// Orion Grid Item
void UOrionGridItem::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UOrionGridItem::SetItemInstance_Implementation(UOrionInventoryItemInstance* InItemInstance, bool bUpdateItemIndex)
{
	UOrionInventoryItemInstance* CachedItemInstance = ItemInstance;

	ItemInstance = InItemInstance;

	if (bUpdateItemIndex)
	{
		UOrionInventoryFunctionLibrary::AssignItemIndex(this, InItemInstance, GridIndex);
	}

	if (InItemInstance == nullptr)
	{
		OnItemRemoved.Broadcast();
		RemoveFromParent();
	}
	else if(CachedItemInstance != InItemInstance)
	{
		OnItemChanged.Broadcast(InItemInstance);
	}
}
