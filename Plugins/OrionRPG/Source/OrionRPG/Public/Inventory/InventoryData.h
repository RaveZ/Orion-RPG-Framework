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

UENUM(BlueprintType)
enum class EOrionInventoryType : uint8
{
	/**Weight based inventory, using metric kg as max capacity*/
	E_WeightBased	UMETA(DisplayName = "Weight Based Inventory"),
	/**
	* Grid based inventory, using grid count as max capacity.
	* Usually being used for spatial inventory.
	*/
	E_GridBased		UMETA(DisplayName = "Grid Based Inventory"),
};

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
	void AssignItemIndex(UOrionInventoryItemInstance* ItemInstance, int32 TileIndex);
	UOrionInventoryItemInstance* AddEntry(UOrionInventoryNode_ItemDefinition* ItemDef);
	void RemoveEntry(UOrionInventoryItemInstance* ItemToRemove);
	void Reset();
	void InitializeFromSaveGame(const struct FOrionInventorySaveData& InventorySaveData);
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
struct FItemInstanceData
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	FGameplayTag Tag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	FText DisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	FText Description;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 StackCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 MaxStackCount;

	UPROPERTY(EditAnywhere, meta = (Units = "Kilograms", UIMin = "0"), Category = "Inventory")
	float Weight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UOrionInventoryNode_ItemDefinition> ItemDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", Instanced)
	TObjectPtr<UOrionInventoryItemFragment> DropItemFragment;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
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

	UPROPERTY(SaveGame)
	int32 TileIndex;

public:
	FOrionInventoryItemSaveData()
	{
		ItemTag = FGameplayTag::EmptyTag;
		TotalStackCount = 0;
	}
};
