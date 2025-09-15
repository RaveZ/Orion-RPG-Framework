// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Inventory/InventoryData.h"
#include "Inventory/OrionInventoryComponent.h"
#include "Inventory/OrionInventoryNode_Item.h"
#include "OrionRPG.h"

//Gameplay Tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Item, "Inventory.Item");

void FInventoryFastArray::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	UOrionInventoryComponent* InvComponent = Cast<UOrionInventoryComponent>(OwnerComponent);
	if (!IsValid(InvComponent)) return;

	for (int32 Index : RemovedIndices)
	{
		InvComponent->OnItemRemoved.Broadcast(Entries[Index].Item);
	}
}

void FInventoryFastArray::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	UOrionInventoryComponent* InvComponent = Cast<UOrionInventoryComponent>(OwnerComponent);
	if (!IsValid(InvComponent)) return;

	for (int32 Index : AddedIndices)
	{
		InvComponent->OnItemAdded.Broadcast(Entries[Index].Item);
	}
}


TArray<UOrionInventoryNode_Item*> FInventoryFastArray::GetAllItems() const
{

	TArray<UOrionInventoryNode_Item*> Results;
	Results.Reserve(Entries.Num());
	for (const auto& Entry : Entries)
	{
		if (!IsValid(Entry.Item)) continue;
		Results.Add(Entry.Item);
	}
	return Results;
}


UOrionInventoryNode_Item* FInventoryFastArray::AddEntry(UOrionInteractionTargetComponent* ItemComponent)
{
	return nullptr;
}

UOrionInventoryNode_Item* FInventoryFastArray::AddEntry(UOrionInventoryNode_Item* Item)
{
	check(OwnerComponent);
	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());

	FInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Item = Item;

	MarkItemDirty(NewEntry);
	return Item;
}

void FInventoryFastArray::RemoveEntry(UOrionInventoryNode_Item* Item)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FInventoryEntry& Entry = *EntryIt;
		if (Entry.Item == Item)
		{
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}
