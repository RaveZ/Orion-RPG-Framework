// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Inventory/InventoryData.h"
#include "Inventory/OrionInventoryComponent.h"
#include "Inventory/OrionInventoryGraph.h"
#include "Inventory/OrionInventoryNode_ItemDefinition.h"
#include "Inventory/OrionInventoryItemInstance.h"
#include "Inventory/OrionInventoryItemFragment.h"
#include "OrionSaveGame.h"
#include "OrionRPG.h"

//////////////////////////////////////////////////////////////////////

// Gameplay Tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Item, "Item");

//////////////////////////////////////////////////////////////////////
// FOrionInventoryEntry

FString FOrionInventoryEntry::GetDebugString() const
{
	/*TSubclassOf<UOrionInventoryItemDefinition> ItemDef;
	if (Instance != nullptr)
	{
		ItemDef = Instance->GetItemDef();
	}

	return FString::Printf(TEXT("%s (%d x %s)"), *GetNameSafe(Instance), StackCount, *GetNameSafe(ItemDef));*/
	return FString();
}

//////////////////////////////////////////////////////////////////////
// FOrionInventoryList
void FOrionInventoryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	UOrionInventoryComponent* InvComponent = Cast<UOrionInventoryComponent>(OwnerComponent);
	if (!IsValid(InvComponent)) return;

	if (!InvComponent->bInitializingSaveGame)
	{
		for (int32 Index : RemovedIndices)
		{
			InvComponent->OnItemRemoved.Broadcast(Entries[Index].ItemInstance);
		}
	}
	
}

void FOrionInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	UOrionInventoryComponent* InvComponent = Cast<UOrionInventoryComponent>(OwnerComponent);
	if (!IsValid(InvComponent)) return;

	if (!InvComponent->bInitializingSaveGame)
	{
		for (int32 Index : AddedIndices)
		{
			InvComponent->OnItemAdded.Broadcast(Entries[Index].ItemInstance);
		}
	}
}

void FOrionInventoryList::UpdateGrid(FIntPoint OldDimension, FIntPoint NewDimension)
{
	UOrionInventoryComponent* InventoryComponent = Cast<UOrionInventoryComponent>(OwnerComponent);

	if (OldDimension == NewDimension) return;

	if (InventoryComponent)
	{
		int32 OldRows = OldDimension.Y;
		int32 OldCols = OldDimension.X;
		int32 NewRows = NewDimension.Y;
		int32 NewCols = NewDimension.X;

		TArray<FOrionInventoryEntry> NewEntries;
		NewEntries.SetNum(NewCols * NewRows);

		// Shift items to new grid, preserving their relative positions as much as possible
		for (int32 i = 0; i < Entries.Num(); ++i)
		{
			int32 OldRow = i / OldCols;
			int32 OldCol = i % OldCols;

			// If the item fits in the new grid, copy it to the same position
			if (OldRow < NewRows && OldCol < NewCols)
			{
				int32 NewIndex = OldRow * NewCols + OldCol;
				NewEntries[NewIndex] = Entries[i];
			}
			// Otherwise, item is dropped (could be handled differently if needed)
		}

		Entries = NewEntries;
		MarkArrayDirty();
	}
	
}

TArray<bool> FOrionInventoryList::GetOccupiedList()
{
	UOrionInventoryComponent* InventoryComp = Cast<UOrionInventoryComponent>(OwnerComponent);

	
	TArray<bool> OccupiedList;
	OccupiedList.SetNum(Entries.Num());
	for (int32 i = 0; i < Entries.Num(); i++)
	{
		if (Entries[i].ItemInstance)
		{
			if (InventoryComp)
			{
				int32 Row = InventoryComp->GetCurrentGridDimension().Y;
				int32 Col = InventoryComp->GetCurrentGridDimension().X;
				FIntPoint ItemSize = Entries[i].ItemInstance->GetItemDef()->GridSize;

				for (int32 Y = 0; Y < ItemSize.Y; Y++)
				{
					for (int32 X = 0; X < ItemSize.X; X++)
					{
						int32 SlotIndex = i + (Y * Col) + X;
						if (OccupiedList.IsValidIndex(SlotIndex))
						{
							OccupiedList[SlotIndex] = true;
						}
					}
				}
			}
		}
	}
	
	return OccupiedList;
}

bool FOrionInventoryList::IsValidSlot(UOrionInventoryNode_ItemDefinition* ItemDef, int32 SlotIndex)
{
	bool bValid = true;
	TArray<bool> OccupiedList = GetOccupiedList();
	UOrionInventoryComponent* InventoryComp = Cast<UOrionInventoryComponent>(OwnerComponent);
	if (InventoryComp && ItemDef)
	{
		int32 Row = InventoryComp->GetCurrentGridDimension().Y;
		int32 Col = InventoryComp->GetCurrentGridDimension().X;
		FIntPoint ItemSize = ItemDef->GridSize;
		int32 StartingIndex = SlotIndex;
		for (int32 Y = 0; Y < ItemSize.Y; Y++)
		{
			for (int32 X = 0; X < ItemSize.X; X++)
			{
				int32 InIndex = StartingIndex + (Y * Col) + X;
				int32 SlotX = (StartingIndex % Col) + X;
				int32 SlotY = (StartingIndex / Col) + Y;
				if (SlotX >= Col || SlotY >= Row)
				{
					bValid = false;
					break;
				}
				if (!Entries.IsValidIndex(InIndex))
				{
					bValid = false;
					break;
				}
				if (OccupiedList[InIndex] == true)
				{
					bValid = false;
					break;
				}
			}
		}
	}
	return bValid;
}

void FOrionInventoryList::Initialize(UOrionInventoryComponent* InOwnerComponent)
{
	OwnerComponent = InOwnerComponent;
	UOrionInventoryGraph* InventoryGraph = InOwnerComponent->InventoryDB;
	if (InventoryGraph && InventoryGraph->InventoryType == EOrionInventoryType::E_GridBased)
	{
		FIntPoint GridDimension = InOwnerComponent->InventoryDB->DefaultGridDimension;
		Entries.SetNum(GridDimension.X * GridDimension.Y);
	}
	MarkArrayDirty();
}

void FOrionInventoryList::SetEntries(TArray<UOrionInventoryItemInstance*> InItemList)
{
	TArray<UOrionInventoryItemInstance*> CurrentItemList = GetAllItems();
	Entries.SetNum(InItemList.Num());
	for (int32 i = 0; i < CurrentItemList.Num(); ++i)
	{
		if (InItemList.IsValidIndex(i))
		{
			if (CurrentItemList[i] == InItemList[i]) continue;
			Entries[i].ItemInstance = InItemList[i];
		}
	}
	MarkArrayDirty();
	return;
}


TArray<UOrionInventoryItemInstance*> FOrionInventoryList::GetAllItems() const
{
	TArray<UOrionInventoryItemInstance*> Results;
	Results.SetNum(Entries.Num());
	for (int32 i = 0; i < Entries.Num(); i++)
	{
		Results[i] = Entries[i].ItemInstance;
	}

	return Results;
}

void FOrionInventoryList::AddEntry(UOrionInventoryItemInstance* ItemInstance)
{
	unimplemented();
}

void FOrionInventoryList::AssignItemIndex(UOrionInventoryItemInstance* ItemInstance, int32 TileIndex)
{
	FIntPoint ItemSize = ItemInstance ? ItemInstance->GetItemDef()->GridSize : FIntPoint(1,1);
	int32 MaxTileIndex = TileIndex + ItemSize.X * ItemSize.Y - 1;
	if (Entries.Num() <= MaxTileIndex)
	{
		Entries.SetNum(MaxTileIndex + 1);
	}
	FOrionInventoryEntry& NewEntry = Entries[TileIndex];
	NewEntry.ItemInstance = ItemInstance;

	MarkItemDirty(NewEntry);
	
}

UOrionInventoryItemInstance* FOrionInventoryList::AddEntry(UOrionInventoryNode_ItemDefinition* ItemDef)
{
	UOrionInventoryItemInstance* NewItemInstance = nullptr;

	check(ItemDef != nullptr);
	check(OwnerComponent);

	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());

	UOrionInventoryGraph* InventoryGraph = OwnerComponent->InventoryDB;
	check(InventoryGraph);

	// Find first empty slot
	int32 EmptyIndex = INDEX_NONE;
	TArray<bool> OccupiedList = GetOccupiedList();
	for (int32 i = 0; i < OccupiedList.Num(); ++i)
	{
		if (OccupiedList[i] == false) // if slot index is not occupied
		{
			if (IsValidSlot(ItemDef, i))
			{
				EmptyIndex = i;
				break;
			}
		}
	}

	switch (InventoryGraph->InventoryType)
	{
		case EOrionInventoryType::E_GridBased:
			if (EmptyIndex == INDEX_NONE)
			{
				OwnerComponent->OnNoRoomForItem.Broadcast(ItemDef);
				UE_LOG(LogTemp, Warning, TEXT("No room for item: %s"), *ItemDef->GetName());
				return nullptr;
			}
			break;
		case EOrionInventoryType::E_WeightBased:
			EmptyIndex = EmptyIndex != INDEX_NONE ? EmptyIndex : Entries.Num();
			if (ItemDef->GridSize.X > OwnerComponent->GetCurrentGridDimension().X)
			{
				UE_LOG(LogTemp, Warning, TEXT("Item %s too large for inventory (GridSize.X: %d, InventoryCols: %d) \n Please adjust inventory max row to fit the item."), *ItemDef->Tag.GetTagName().ToString(), ItemDef->GridSize.X, OwnerComponent->GetCurrentGridDimension().X);
				return nullptr;
			}
			break;
		default:
			break;
	}

	NewItemInstance = NewObject<UOrionInventoryItemInstance>(OwnerComponent->GetOwner());
	NewItemInstance->SetItemDef(ItemDef);
	ItemDef->LoadPrerequisites();
	for (UOrionInventoryItemFragment* Fragment : ItemDef->Fragments)
	{
		if (Fragment != nullptr)
		{
			Fragment->OnInstanceCreated(NewItemInstance);
		}
	}
	if (ItemDef->DropItemFragment)
	{
		ItemDef->DropItemFragment->OnInstanceCreated(NewItemInstance);
	}

	AssignItemIndex(NewItemInstance, EmptyIndex);

	MarkItemDirty(Entries[EmptyIndex]);


	return NewItemInstance;
}

void FOrionInventoryList::RemoveEntry(UOrionInventoryItemInstance* ItemToRemove)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FOrionInventoryEntry& Entry = *EntryIt;
		if (Entry.ItemInstance == ItemToRemove)
		{
			Entry.ItemInstance = nullptr;
		}
	}
	MarkArrayDirty();
}

void FOrionInventoryList::Reset()
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		EntryIt.RemoveCurrent();
	}
	MarkArrayDirty();
}

void FOrionInventoryList::InitializeFromSaveGame(const FOrionInventorySaveData& InventorySaveData)
{
	check(OwnerComponent);

	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());

	UOrionInventoryComponent* InvComponent = Cast<UOrionInventoryComponent>(OwnerComponent);

	Entries.SetNum(InventorySaveData.SavedInventoryItems.Num());
	for (int i = 0; i < InventorySaveData.SavedInventoryItems.Num(); i++)
	{
		if (InventorySaveData.SavedInventoryItems.IsValidIndex(i))
		{
			const FOrionInventoryItemSaveData& ItemSaveData = InventorySaveData.SavedInventoryItems[i];
			UOrionInventoryNode_ItemDefinition* ItemDef = InvComponent->FindItemDefinition(ItemSaveData.ItemTag);

			if (ItemDef)
			{
				FOrionInventoryEntry& NewEntry = Entries[i];
				NewEntry.ItemInstance = NewObject<UOrionInventoryItemInstance>(OwnerComponent->GetOwner());
				NewEntry.ItemInstance->SetItemDef(ItemDef);
				NewEntry.ItemInstance->SetTotalStackCount(ItemSaveData.TotalStackCount);

				ItemDef->LoadPrerequisites();
				for (UOrionInventoryItemFragment* Fragment : ItemDef->Fragments)
				{
					if (Fragment != nullptr)
					{
						Fragment->OnInstanceCreated(NewEntry.ItemInstance);
					}
				}
				if (ItemDef->DropItemFragment)
				{
					ItemDef->DropItemFragment->OnInstanceCreated(NewEntry.ItemInstance);
				}
			}
		}
	}
	
	

}

UOrionInventoryItemInstance* FOrionInventoryList::FindFirstItemByTag(const FGameplayTag& ItemTag)
{
	auto* FoundItem = Entries.FindByPredicate([ItemTag = ItemTag](const FOrionInventoryEntry& Entry)
		{
			return IsValid(Entry.ItemInstance) && IsValid(Entry.ItemInstance->GetItemDef()) && Entry.ItemInstance->GetItemDef()->Tag.MatchesTagExact(ItemTag);
		});
	return FoundItem ? FoundItem->ItemInstance : nullptr;
}


//////////////////////////////////////////////////////////////////////
// 
// FOrionEquippedItemEntry
FString FOrionEquippedItemEntry::GetDebugString() const
{
	return FString();
}


//////////////////////////////////////////////////////////////////////
// 
// FOrionEquippedItemEntry

void FOrionEquipmentList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
}

void FOrionEquipmentList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
}

void FOrionEquipmentList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
}
