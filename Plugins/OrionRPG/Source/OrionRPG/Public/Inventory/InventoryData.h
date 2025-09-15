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

USTRUCT(BlueprintType)
struct FInventoryItemDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Inventory.Item"))
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<AActor> ActorClass;


public:
	FInventoryItemDefinition()
	{
		Icon = nullptr;
		MaxCount = 1;
		ActorClass = nullptr;
	}

};



USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	FGameplayTag Tag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	FText Description;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	int32 MaxCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	int32 Count;


public:
	FInventoryItem()
	{
		Icon = nullptr;
		MaxCount = 0;
		Count = 0;
	}


};