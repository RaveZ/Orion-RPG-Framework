// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "NativeGameplayTags.h"
#include "InventoryData.generated.h"

class UOrionInventoryNode_ItemDefinition;
class UOrionInventoryComponent;
class UOrionInteractionTargetComponent;


//Gameplay Tags
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item);


USTRUCT(BlueprintType)
struct FOrionInventoryEntry : public FFastArraySerializerItem
{
    GENERATED_USTRUCT_BODY()
	FOrionInventoryEntry() {}
	FString GetDebugString() const;

private:
	friend struct FOrionInventoryList;
	friend UOrionInventoryComponent;

	UPROPERTY()
	TObjectPtr<UOrionInventoryNode_ItemDefinition> Item = nullptr;

	UPROPERTY()
	int32 StackCount = 0;

	UPROPERTY(NotReplicated)
	int32 LastObservedCount = INDEX_NONE;
};

/** List of inventory items */
USTRUCT(BlueprintType)
struct FOrionInventoryList : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	FOrionInventoryList() : OwnerComponent(nullptr) {}
	FOrionInventoryList(UOrionInventoryComponent* InOwnerComponent) : OwnerComponent(InOwnerComponent) {}

	TArray<UOrionInventoryNode_ItemDefinition*> GetAllItems() const;

	// FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	// End of FFastArraySerializer contract			

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FOrionInventoryEntry, FOrionInventoryList>(Entries, DeltaParams, *this);
	}

	UOrionInventoryNode_ItemDefinition* AddEntry(UOrionInteractionTargetComponent* ItemComponent);
	UOrionInventoryNode_ItemDefinition* AddEntry(UOrionInventoryNode_ItemDefinition* Item);
	void RemoveEntry(UOrionInventoryNode_ItemDefinition* Item);

private:
	friend UOrionInventoryComponent;

	// Replicated list of items
	UPROPERTY()
	TArray<FOrionInventoryEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UOrionInventoryComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FOrionInventoryList> : public TStructOpsTypeTraitsBase2<FOrionInventoryList>
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