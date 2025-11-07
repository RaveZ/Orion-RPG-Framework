// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Inventory/InventoryData.h"
#include "Inventory/OrionInventoryComponent.h"
#include "Inventory/OrionInventoryNode_ItemDefinition.h"
#include "Inventory/OrionInventoryItemInstance.h"
#include "Inventory/OrionInventoryItemFragment.h"
#include "OrionSaveGame.h"
#include "OrionRPG.h"

//Gameplay Tags
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


TArray<UOrionInventoryItemInstance*> FOrionInventoryList::GetAllItems() const
{

	TArray<UOrionInventoryItemInstance*> Results;
	Results.SetNum(Entries.Num());
	for (int32 i = 0; i < Entries.Num(); i++)
	{
		Results[i] = Entries[i].ItemInstance;
	}

	//Results.Reserve(Entries.Num());
	//for (const FOrionInventoryEntry& Entry : Entries)
	//{
	//	if (Entry.ItemInstance != nullptr) //@TODO: Would prefer to not deal with this here and hide it further?
	//	{
	//		Results.Add(Entry.ItemInstance);
	//	}
	//}
	return Results;
}

void FOrionInventoryList::AddEntry(UOrionInventoryItemInstance* ItemInstance)
{
	unimplemented();
}

void FOrionInventoryList::AssignItemIndex(UOrionInventoryItemInstance* ItemInstance, int32 TileIndex)
{
	if (Entries.Num() <= TileIndex)
	{
		Entries.SetNum(TileIndex+1);
	}
	FOrionInventoryEntry& NewEntry = Entries[TileIndex];
	NewEntry.ItemInstance = ItemInstance;
	
}

UOrionInventoryItemInstance* FOrionInventoryList::AddEntry(UOrionInventoryNode_ItemDefinition* ItemDef)
{
	UOrionInventoryItemInstance* Result = nullptr;

	check(ItemDef != nullptr);
	check(OwnerComponent);

	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());
	// Find first empty slot
	int32 EmptyIndex = INDEX_NONE;
	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		if (Entries[i].ItemInstance == nullptr)
		{
			EmptyIndex = i;
			break;
		}
	}

	FOrionInventoryEntry& NewEntry = EmptyIndex != INDEX_NONE ? Entries[EmptyIndex] : Entries.AddDefaulted_GetRef();

	NewEntry.ItemInstance = NewObject<UOrionInventoryItemInstance>(OwnerComponent->GetOwner());
	NewEntry.ItemInstance->SetItemDef(ItemDef);
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
	Result = NewEntry.ItemInstance;

	MarkItemDirty(NewEntry);

	return Result;
}

void FOrionInventoryList::RemoveEntry(UOrionInventoryItemInstance* ItemToRemove)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FOrionInventoryEntry& Entry = *EntryIt;
		if (Entry.ItemInstance == ItemToRemove)
		{
			Entry.ItemInstance = nullptr;
			MarkArrayDirty();
		}
	}
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
