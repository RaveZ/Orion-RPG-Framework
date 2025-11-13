// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "UI/Foundation/OrionGridBase.h"
#include "Inventory/OrionInventoryItemInstance.h"
#include "Inventory/OrionInventoryFunctionLibrary.h"
#include "CommonActionWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OrionGridBase)

void UOrionGridBase::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UOrionGridSlot::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UOrionGridItem::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UOrionGridItem::SetItemInstance_Implementation(UOrionInventoryItemInstance* InItemInstance, bool bUpdateItemIndex)
{
	ItemInstance = InItemInstance;
	if (bUpdateItemIndex)
	{
		UOrionInventoryFunctionLibrary::AssignItemIndex(this, InItemInstance, GridIndex);
	}
}
