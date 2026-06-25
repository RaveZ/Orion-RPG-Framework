// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "NativeGameplayTags.h"
#include "AbilitySystem/OrionAbilitySet.h"
#include "Engine/Texture2D.h"
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
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_Equipment);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_EquipSlot_Head);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_EquipSlot_Chest);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_EquipSlot_Legs);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_EquipSlot_Feet);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_EquipSlot_Hands);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_EquipSlot_RightHandWeapon);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_EquipSlot_LeftHandWeapon);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_EquipSlot_Accessory1);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_EquipSlot_Accessory2);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Currency_Gold);

UENUM(BlueprintType)
enum class EDropItemBehavior : uint8
{
	/**Drop the item as pickupable*/
	E_DropAsPickupable	UMETA(DisplayName = "Drop as Pickupable"),
	/**
	* Drop the item as lootable container.
	*/
	E_DropAsLootable		UMETA(DisplayName = "Drop as Lootable"),
};


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
		bSetLeaderComponent = false;
	}

public:
	UPROPERTY(BlueprintReadOnly, Category = Equipment)
	TObjectPtr<AActor> SpawnedActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Equipment)
	TSoftClassPtr<AActor> ActorToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Equipment)
	FName AttachSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Equipment)
	FTransform AttachTransform;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Equipment)
	bool bSetLeaderComponent;
};

//////////////////////////////////////////////////////////////////////

// Equipment Definition
USTRUCT(BlueprintType)
struct FOrionEquipmentDefinition
{
	GENERATED_BODY()

	FOrionEquipmentDefinition();
	
	UPROPERTY(BlueprintReadWrite, Category = Equipment, meta = (Categories = "Item"))
	FGameplayTag OwningItem;
	
	
	//Which equipment slot to assign to
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Equipment)
	TSubclassOf<UAnimInstance> OverlayAnimationBlueprint;
	
	//Which equipment slot to assign to
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Equipment, meta=(Categories="EquipSlot"))
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

// Pickup Definition
USTRUCT(BlueprintType)
struct FOrionPickupDefinition
{
	GENERATED_BODY()

	FOrionPickupDefinition()
	{
		Count = 1;
	}
public:

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = Pickup, meta = (Categories = "Item"))
	FGameplayTag ItemTag;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = Pickup)
	int32 Count;

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
	FOrionAbilitySet_GrantedHandles GrantedHandles;
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

	UOrionEquipmentInstance* AddEntry(FOrionEquipmentDefinition EquipmentDefinition);//add new entry
	UOrionEquipmentInstance* SetEntry(FOrionEquipmentDefinition EquipmentDefinition);//set existing entry
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
		Weight = 0.0f;
	}


};

//////////////////////////////////////////////////////////////////////

// Currency Data
USTRUCT(BlueprintType)
struct FOrionCurrencyData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta=(Categories="Currency"))
	FGameplayTag CurrencyTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	FText CurrencyName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	int32 Value;

public:
	FOrionCurrencyData()
	{
		Value = 0;
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
		TileIndex = 0;
	}
};