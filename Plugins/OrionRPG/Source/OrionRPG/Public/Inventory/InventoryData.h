// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "NativeGameplayTags.h"
#include "InventoryData.generated.h"

class UOrionInventoryNode_ItemDefinition;
class UOrionInventoryItemInstance;
class UOrionInventoryItemFragment;
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
	TObjectPtr<UOrionInventoryItemInstance> ItemInstance = nullptr;
};

/** List of inventory items */
USTRUCT(BlueprintType)
struct FOrionInventoryList : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	FOrionInventoryList() 
		: OwnerComponent(nullptr) 
	{
	}

	FOrionInventoryList(UOrionInventoryComponent* InOwnerComponent) 
		: OwnerComponent(InOwnerComponent) 
	{
	}

	TArray<UOrionInventoryItemInstance*> GetAllItems() const;

public:
	// FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	// End of FFastArraySerializer contract			

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FOrionInventoryEntry, FOrionInventoryList>(Entries, DeltaParams, *this);
	}

	void AddEntry(UOrionInventoryItemInstance* ItemInstance);
	UOrionInventoryItemInstance* AddEntry(UOrionInventoryNode_ItemDefinition* ItemDef);
	void RemoveEntry(UOrionInventoryItemInstance* ItemToRemove);
	void Reset();
	UOrionInventoryItemInstance* FindFirstItemByTag(const FGameplayTag& ItemTag);

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

	UPROPERTY(EditAnywhere, meta = (Units = "Kilograms", UIMin = "0"))
	float Weight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<AActor> ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fragments", Instanced)
	TArray<TObjectPtr<UOrionInventoryItemFragment>> Fragments;

public:
	FInventoryItemDefinition()
	{
		Tag = TAG_Item;
		Weight = 0.0f;
		Icon = nullptr;
		MaxCount = 1;
		ActorClass = nullptr;
	}

};


USTRUCT(BlueprintType)
struct FItemInstanceData
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
	int32 StackCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	int32 MaxStackCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UOrionInventoryNode_ItemDefinition> ItemDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", Instanced)
	TObjectPtr<UOrionInventoryItemFragment> DropItemFragment;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TArray<TObjectPtr<UOrionInventoryItemFragment>> Fragments;

public:
	FItemInstanceData()
	{
		Icon = nullptr;
		MaxStackCount = 0;
		StackCount = 0;
	}


};

/** Inventory  Data To Be Saved */
USTRUCT()
struct FOrionInventoryItemSaveData
{
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame)
	FGameplayTag ItemTag;

	UPROPERTY(SaveGame)
	int32 TotalStackCount;


public:
	FOrionInventoryItemSaveData()
	{
		ItemTag = FGameplayTag::EmptyTag;
		TotalStackCount = 0;
	}
};
