// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Inventory/InventoryData.h"
#include "Inventory/OrionInventoryComponent.h"
#include "Inventory/OrionInventoryNode_ItemDefinition.h"
#include "Inventory/OrionInventoryItemInstance.h"
#include "Inventory/OrionInventoryItemFragment.h"
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
	Results.Reserve(Entries.Num());
	for (const FOrionInventoryEntry& Entry : Entries)
	{
		if (Entry.ItemInstance != nullptr) //@TODO: Would prefer to not deal with this here and hide it further?
		{
			Results.Add(Entry.ItemInstance);
		}
	}
	return Results;
}

void FOrionInventoryList::AddEntry(UOrionInventoryItemInstance* ItemInstance)
{
	unimplemented();
}

UOrionInventoryItemInstance* FOrionInventoryList::AddEntry(UOrionInventoryNode_ItemDefinition* ItemDef)
{
	UOrionInventoryItemInstance* Result = nullptr;

	check(ItemDef != nullptr);
	check(OwnerComponent);

	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());


	FOrionInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.ItemInstance = NewObject<UOrionInventoryItemInstance>(OwnerComponent->GetOwner());  //@TODO: Using the actor instead of component as the outer due to UE-127172
	NewEntry.ItemInstance->SetItemDef(ItemDef);
	for (UOrionInventoryItemFragment* Fragment : ItemDef->Fragments)
	{
		if (Fragment != nullptr)
		{
			Fragment->OnInstanceCreated(NewEntry.ItemInstance);
		}
	}
	Result = NewEntry.ItemInstance;

	//const UOrionInventoryItemDefinition* ItemCDO = GetDefault<UOrionInventoryItemDefinition>(ItemDef);
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
			EntryIt.RemoveCurrent();
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

UOrionInventoryItemInstance* FOrionInventoryList::FindFirstItemByTag(const FGameplayTag& ItemTag)
{
	auto* FoundItem = Entries.FindByPredicate([ItemTag = ItemTag](const FOrionInventoryEntry& Entry)
		{
			return IsValid(Entry.ItemInstance) && IsValid(Entry.ItemInstance->GetItemDef()) && Entry.ItemInstance->GetItemDef()->Tag.MatchesTagExact(ItemTag);
		});
	return FoundItem ? FoundItem->ItemInstance : nullptr;
}
