// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "UI/Foundation/OrionGridBase.h"
#include "UI/Foundation/OrionGridContainerBase.h"
#include "Inventory/OrionInventoryItemInstance.h"
#include "Inventory/OrionInventoryComponent.h"
#include "Inventory/OrionInventoryGraph.h"
#include "Inventory/OrionInventoryNode_ItemDefinition.h"
#include "UI/OrionUIManagerSubsystem.h"
#include "Inventory/OrionInventoryFunctionLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "CommonActionWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OrionGridBase)

//////////////////////////////////////////////////////////////////////

// Orion Grid Base
void UOrionGridBase::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UOrionGridBase::InitializeGrid(UOrionGridContainerBase* InGridContainer)
{
	OwningGridContainer = InGridContainer;
}

int32 UOrionGridBase::GetIndexFromPosition(int32 PosX, int32 PosY, int32 Columns)
{
	return PosX + (PosY * Columns);
}

//////////////////////////////////////////////////////////////////////

// Orion Grid Slot
void UOrionGridSlot::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UOrionGridSlot::InitializeGrid(UOrionGridContainerBase* InGridContainer)
{
	Super::InitializeGrid(InGridContainer);
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

void UOrionGridItem::NativeConstruct()
{
	Super::NativeConstruct();
	

	//DragTargetPosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
}

void UOrionGridItem::InitializeGrid(UOrionGridContainerBase* InGridContainer)
{
	Super::InitializeGrid(InGridContainer);
}


void UOrionGridItem::SetItemInstance_Implementation(UOrionInventoryItemInstance* InItemInstance, bool bUpdateItemIndex)
{
	if (ItemInstance == InItemInstance) return;
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
		UpdateItemSize(InItemInstance);
	}
}

void UOrionGridItem::UpdateItemSize(UOrionInventoryItemInstance* InItemInstance)
{
	if (InItemInstance)
	{
		UOrionInventoryGraph* InventoryDB = UOrionInventoryFunctionLibrary::GetInventoryDB(this);
		if (InventoryDB)
		{
			float CachedGridSize = InventoryDB->GridSize;
			FIntPoint InItemDimension = InItemInstance->GetItemDef()->GridSize;
			float SizeX = CachedGridSize * InItemDimension.X;
			float SizeY = CachedGridSize * InItemDimension.Y;

			ItemDimension = InItemDimension;
			SetDesiredSizeInViewport(FVector2D(SizeX, SizeY));

			if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
			{
				CanvasSlot->SetSize(FVector2D(SizeX, SizeY));
			}
		}
	}
}

bool UOrionGridItem::IsValidSlot(UOrionGridSlot* InOtherSlot)
{
	bool bValid = true;
	UOrionInventoryComponent* InventoryComp = UOrionInventoryFunctionLibrary::GetInventoryComponent(this);
	TArray<UOrionGridSlot*> ActiveSlots = InOtherSlot->OwningGridContainer->GridSlots;
	if (InventoryComp && InOtherSlot)
	{
		int32 Row = InventoryComp->GetCurrentGridDimension().Y;
		int32 Col = InventoryComp->GetCurrentGridDimension().X;
		FIntPoint ItemSize = ItemInstance->GetItemDef()->GridSize;
		int32 StartingIndex = InOtherSlot->GridIndex;
		for (int32 Y = 0; Y < ItemSize.Y; Y++)
		{
			for (int32 X = 0; X < ItemSize.X; X++)
			{
				int32 SlotIndex = StartingIndex + (Y * Col) + X;
				int32 SlotX = (StartingIndex % Col) + X;
				int32 SlotY = (StartingIndex / Col) + Y;
				if (SlotX >= Col || SlotY >= Row)
				{
					bValid = false;
					break;
				}
				if (!ActiveSlots.IsValidIndex(SlotIndex))
				{
					bValid = false;
					break;
				}
				if (ActiveSlots[SlotIndex]->GridItem &&
					ActiveSlots[SlotIndex]->GridItem->ItemInstance != ItemInstance)
				{
					bValid = false;
					break;
				}
			}
		}
	}
	return bValid;
}

