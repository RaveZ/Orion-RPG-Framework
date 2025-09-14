// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "NativeGameplayTags.h"
#include "InventoryData.generated.h"

class UOrionInventoryNode_Item;
class UOrionInventoryComponent;
class UOrionInteractionTargetComponent;


//Gameplay Tags
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item);


USTRUCT(BlueprintType)
struct FInventoryEntry : public FFastArraySerializerItem
{
    GENERATED_USTRUCT_BODY()
	FInventoryEntry() {}

private:
	friend struct FInventoryFastArray;
	friend UOrionInventoryComponent;

	UPROPERTY()
	TObjectPtr<UOrionInventoryNode_Item> Item = nullptr;

};

/** List of inventory items */
USTRUCT(BlueprintType)
struct FInventoryFastArray : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	FInventoryFastArray() : OwnerComponent(nullptr) {}
	FInventoryFastArray(UOrionInventoryComponent* InOwnerComponent) : OwnerComponent(InOwnerComponent) {}

	TArray<UOrionInventoryNode_Item*> GetAllItems() const;

	// FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	// End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FInventoryEntry, FInventoryFastArray>(Entries, DeltaParams, *this);
	}

	UOrionInventoryNode_Item* AddEntry(UOrionInteractionTargetComponent* ItemComponent);
	UOrionInventoryNode_Item* AddEntry(UOrionInventoryNode_Item* Item);
	void RemoveEntry(UOrionInventoryNode_Item* Item);

private:
	friend UOrionInventoryComponent;

	// Replicated list of items
	UPROPERTY()
	TArray<FInventoryEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UOrionInventoryComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FInventoryFastArray> : public TStructOpsTypeTraitsBase2<FInventoryFastArray>
{
	enum { WithNetDeltaSerializer = true };
};



