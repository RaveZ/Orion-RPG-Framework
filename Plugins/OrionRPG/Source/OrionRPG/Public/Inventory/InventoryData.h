// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "NativeGameplayTags.h"
#include "Engine/Texture2D.h"
#include "AbilitySystem/AbilityData.h"
#include "InventoryData.generated.h"

class UOrionInventoryNode_ItemDefinition;
class UOrionInventoryItemInstance;
class UOrionEquipmentInstance;
class UOrionInventoryItemFragment;
class UOrionInventoryComponent;
class UOrionAbilitySystemComponent;
class UOrionInteractionTargetComponent;



//////////////////////////////////////////////////////////////////////

// Gameplay Tags
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Equipment_Slot);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Equipment_Slot_Head);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Equipment_Slot_Chest);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Equipment_Slot_Legs);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Equipment_Slot_Feet);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Equipment_Slot_Hands);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Equipment_Slot_RightHandWeapon);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Equipment_Slot_LeftHandWeapon);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Equipment_Slot_Accessory1);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Equipment_Slot_Accessory2);


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

//////////////////////////////////////////////////////////////////////
// 
// FOrionInventoryEntry
USTRUCT(BlueprintType)
struct FOrionInventoryEntry : public FFastArraySerializerItem
{
    GENERATED_BODY()
	FOrionInventoryEntry() {}
	FString GetDebugString() const;

private:
	friend struct FOrionInventoryList;
	friend UOrionInventoryComponent;

	UPROPERTY()
	TObjectPtr<UOrionInventoryItemInstance> ItemInstance = nullptr;
};

//////////////////////////////////////////////////////////////////////
// 
// FOrionInventoryList
USTRUCT(BlueprintType)
struct FOrionInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

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
	void Initialize(UOrionInventoryComponent* InOwnerComponent);
	// FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	// End of FFastArraySerializer contract			

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FOrionInventoryEntry, FOrionInventoryList>(Entries, DeltaParams, *this);
	}

	void SetEntries(TArray<UOrionInventoryItemInstance*> InItemList);
	void AddEntry(UOrionInventoryItemInstance* ItemInstance);
	void AssignItemIndex(UOrionInventoryItemInstance* ItemInstance, int32 TileIndex);
	UOrionInventoryItemInstance* AddEntry(UOrionInventoryNode_ItemDefinition* ItemDef);
	void RemoveEntry(UOrionInventoryItemInstance* ItemToRemove);
	void Reset();
	void InitializeFromSaveGame(const struct FOrionInventorySaveData& InventorySaveData);
	UOrionInventoryItemInstance* FindFirstItemByTag(const FGameplayTag& ItemTag);
	void UpdateGrid(FIntPoint OldDimension, FIntPoint NewDimension);
	TArray<bool> GetOccupiedList();

	bool IsValidSlot(UOrionInventoryNode_ItemDefinition* ItemDef, int32 SlotIndex);

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

//////////////////////////////////////////////////////////////////////

// Equipment Actor Data
USTRUCT(BlueprintType)
struct FOrionEquipmentActorData
{
	GENERATED_BODY()

	FOrionEquipmentActorData()
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Equipment)
	TSoftClassPtr<AActor> ActorToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Equipment)
	FName AttachSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Equipment)
	FTransform AttachTransform;
};

//////////////////////////////////////////////////////////////////////

// Equipment Definition
USTRUCT(BlueprintType)
struct FOrionEquipmentDefinition
{
	GENERATED_BODY()

	FOrionEquipmentDefinition();

	//Which equipment slot to assign to
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Equipment, meta=(Categories="Equipment.Slot"))
	FGameplayTag EquipSlot;

	// Class to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Equipment)
	TSubclassOf<UOrionEquipmentInstance> InstanceType;

	// Gameplay ability sets to grant when this is equipped
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Equipment)
	TArray<TObjectPtr<const UOrionAbilitySet>> AbilitySetsToGrant;

	// Actors to spawn on the pawn when this is equipped
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Equipment)
	TArray<FOrionEquipmentActorData> ActorsToSpawn;
};


//////////////////////////////////////////////////////////////////////
// 
// FOrionEquippedItemEntry
USTRUCT(BlueprintType)
struct FOrionEquippedItemEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()
	FOrionEquippedItemEntry() {}
	FString GetDebugString() const;

private:
	friend struct FOrionEquipmentList;
	friend UOrionInventoryComponent;

	UPROPERTY()
	FGameplayTag Slot;


	UPROPERTY()
	TObjectPtr<UOrionEquipmentInstance> Instance = nullptr;

	// Authority-only list of granted handles
	UPROPERTY(NotReplicated)
	FOrionGrantedHandlesData GrantedHandles;
};


//////////////////////////////////////////////////////////////////////
// 
// FOrionEquippedItemEntry
USTRUCT(BlueprintType)
struct FOrionEquipmentList : public FFastArraySerializer
{

	GENERATED_BODY()

	FOrionEquipmentList()
		: OwnerComponent(nullptr)
	{
	}
	FOrionEquipmentList(UOrionInventoryComponent* InOwnerComponent)
		: OwnerComponent(InOwnerComponent)
	{
	}

	TArray<UOrionEquipmentInstance*> GetAllEquippedItems() const;

public:
	void Initialize(UOrionInventoryComponent* InOwnerComponent);
	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FOrionEquippedItemEntry, FOrionEquipmentList>(Entries, DeltaParms, *this);
	}

	UOrionEquipmentInstance* AddEntry(FOrionEquipmentDefinition EquipmentDefinition);
	UOrionEquipmentInstance* SetEntry(FOrionEquipmentDefinition EquipmentDefinition);
	UOrionEquipmentInstance* CreateEquipmentInstance(FOrionEquippedItemEntry& InEntry, FOrionEquipmentDefinition EquipmentDefinition);
	void RemoveEntry(UOrionEquipmentInstance* Instance);

	FOrionEquippedItemEntry& FindFirstEntryBySlot(bool& bFound, FGameplayTag SlotTag);
	UOrionEquipmentInstance* FindFirstEquipmentBySlot(const FGameplayTag& SlotTag);
		
	void Reset();
private:
	UOrionAbilitySystemComponent* GetAbilitySystemComponent() const; 

	friend UOrionInventoryComponent;

private:
	// Replicated list of equipment entries
	UPROPERTY()
	TArray<FOrionEquippedItemEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UOrionInventoryComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FOrionEquipmentList> : public TStructOpsTypeTraitsBase2<FOrionEquipmentList>
{
	enum { WithNetDeltaSerializer = true };
};


//////////////////////////////////////////////////////////////////////

// Item Instance Data
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

//////////////////////////////////////////////////////////////////////

// Inventory Save Data
USTRUCT()
struct FOrionInventoryItemSaveData
{
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame)
	FGameplayTag ItemTag;

	UPROPERTY(SaveGame)
	bool bIsEquipped;

	UPROPERTY(SaveGame)
	int32 TotalStackCount;

	UPROPERTY(SaveGame)
	int32 TileIndex;

public:
	FOrionInventoryItemSaveData()      
	{
		bIsEquipped = false;
		ItemTag = FGameplayTag::EmptyTag;
		TotalStackCount = 0;
	}
};