// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Inventory/InventoryData.h"
#include "Inventory/OrionInventoryComponent.h"
#include "Inventory/OrionInventoryNode_ItemDefinition.h"
#include "OrionRPG.h"

//Gameplay Tags
UE_DEFINE_GAMEPLAY_TAG(TAG_Item, "Inventory.Item");

//////////////////////////////////////////////////////////////////////
// FOrionInventoryEntry

FString FOrionInventoryEntry::GetDebugString() const
{
	/*TSubclassOf<ULyraInventoryItemDefinition> ItemDef;
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

	for (int32 Index : RemovedIndices)
	{
		InvComponent->OnItemRemoved.Broadcast(Entries[Index].Item);
	}
}

void FOrionInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	UOrionInventoryComponent* InvComponent = Cast<UOrionInventoryComponent>(OwnerComponent);
	if (!IsValid(InvComponent)) return;

	for (int32 Index : AddedIndices)
	{
		InvComponent->OnItemAdded.Broadcast(Entries[Index].Item);
	}
}


TArray<UOrionInventoryNode_ItemDefinition*> FOrionInventoryList::GetAllItems() const
{

	TArray<UOrionInventoryNode_ItemDefinition*> Results;
	Results.Reserve(Entries.Num());
	for (const auto& Entry : Entries)
	{
		if (!IsValid(Entry.Item)) continue;
		Results.Add(Entry.Item);
	}
	return Results;
}


UOrionInventoryNode_ItemDefinition* FOrionInventoryList::AddEntry(UOrionInteractionTargetComponent* ItemComponent)
{
	check(OwnerComponent);
	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());
	UOrionInventoryComponent* IC = Cast<UOrionInventoryComponent>(OwnerComponent);
	if (!IsValid(IC)) return nullptr;

	FOrionInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	//NewEntry.Item = ItemComponent->GetItemManifest().Manifest(OwningActor);

	//IC->AddRepSubObj(NewEntry.Item);
	MarkItemDirty(NewEntry);

	return NewEntry.Item;
}

UOrionInventoryNode_ItemDefinition* FOrionInventoryList::AddEntry(UOrionInventoryNode_ItemDefinition* Item)
{
	check(OwnerComponent);
	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());

	FOrionInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Item = Item;

	MarkItemDirty(NewEntry);
	return Item;
}

void FOrionInventoryList::RemoveEntry(UOrionInventoryNode_ItemDefinition* Item)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FOrionInventoryEntry& Entry = *EntryIt;
		if (Entry.Item == Item)
		{
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}
