// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Inventory/InventoryData.h"
#include "Components/ActorComponent.h"
#include "OrionSaveGame.h"
#include "OrionInventoryComponent.generated.h"

class AOrionActorViewer;
class AOrionLootable;
class UOrionInventoryGraph;
class UOrionInventoryNode_ItemDefinition;
class UOrionInventoryItemInstance;
class UOrionEquipmentInstance;
class APlayerController;
struct FNetDeltaSerializeInfo;
struct FReplicationFlags;
struct FFrame;

struct FInventoryItem;

//Inventory Delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInventorySignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FInventoryItemAdded, UOrionInventoryItemInstance*, Item, FItemInstanceData, ItemData, int, AddedCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInventoryItemSignature, UOrionInventoryItemInstance*, Item, FItemInstanceData, ItemData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryNoRoomForItem, UOrionInventoryNode_ItemDefinition*, InItemDefinition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInventoryInspectItem, UOrionInventoryItemInstance*, Item, TSoftClassPtr<AActor>, ActorToInspect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInventoryGridDimensionChanged, FIntPoint, InOldDimension, FIntPoint, InNewDimension);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryMaxWeightChanged, float, InMaxWeight);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryEncumberedStateChanged, bool, bEncumbered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryOnLootItem, UOrionInteractionTargetComponent_LootItem*, InLootComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrencyDataChanged, FOrionCurrencyData, InCurrencyData);


//Equipment Delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEquipmentSignature, UOrionEquipmentInstance*, EquipmentInstance);

UCLASS(meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable, HideDropdown, ClassGroup = "OrionRPG", DisplayName = "Orion Inventory Component")
class ORIONRPG_API UOrionInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOrionInventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**Inventory database*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InventoryComponent")
	TObjectPtr<UOrionInventoryGraph> InventoryDB;

	//We cache the OwningController, we won't cache pawn as this might change
	UPROPERTY(BlueprintReadOnly, Category = "InventoryComponent")
		TObjectPtr<APlayerController> OwningController;

	UPROPERTY(BlueprintReadOnly, Category = "InventoryComponent")
		TObjectPtr<AOrionActorViewer> ItemViewer;

	UPROPERTY(BlueprintReadOnly, Category = "InventoryComponent")
		TObjectPtr<AOrionActorViewer> CharacterViewer;

	UPROPERTY(BlueprintReadOnly, Category = "InventoryComponent")
		TObjectPtr< UOrionInventoryItemInstance> InspectedItem;

	UPROPERTY(BlueprintReadOnly, Category = "InventoryComponent")
		TArray<FOrionCurrencyData> CurrencyList;

	
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
	FInventoryItemAdded OnItemAdded;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
		FInventoryItemSignature OnItemRemoved;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
		FInventoryItemSignature OnItemConsumed;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
		FInventoryItemSignature OnItemStackChanged;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
		FInventoryMaxWeightChanged OnMaxWeightChanged;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
	FInventoryGridDimensionChanged OnGridDimensionChanged;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
	FInventoryNoRoomForItem OnNoRoomForItem;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
		FInventoryEncumberedStateChanged OnEncumberedStateChanged;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
		FInventoryInspectItem OnInspectItem;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
		FEquipmentSignature OnItemEquipped;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
		FEquipmentSignature OnItemUnequipped;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
		FInventoryOnLootItem OnLootItem;


	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
		FOnCurrencyDataChanged OnCurrencyDataChanged;

public:
	UFUNCTION()
	virtual void GridDimensionChanged(FIntPoint OldDimension, FIntPoint NewDimension);
	 
private:
	UPROPERTY(Replicated)
	FOrionInventoryList InventoryList;

	UPROPERTY(Replicated)
	FOrionEquipmentList EquipmentList;

	/**Max weight for this inventory*/
	float MaxWeight;

	/**Current weight for this inventory*/
	float CurrentWeight;

	FIntPoint CurrentGridDimension;

	UPROPERTY()
	TObjectPtr<AOrionLootable> CurrentDroppedItem;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitializeDelegate();

	//~UObject interface
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	//~End of UObject interface
	//~UActorComponent interface

	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
	virtual void ReadyForReplication() override;
	//~End of UActorComponent interface
public:
	UFUNCTION(BlueprintPure , Category = "InventoryComponent")
	float GetCurrentWeight();

	UFUNCTION(BlueprintPure, Category = "InventoryComponent")
	float GetMaxWeight();

	UFUNCTION(BlueprintPure, Category = "InventoryComponent")
	FIntPoint GetCurrentGridDimension();

	/**
	 * Add Item to Inventory
	 *
	 * @param ItemTag:			The Item Tag to add
	 * @param StackCount:		The Stack Count to add
	 * 
	 * @return	StackRemainder:	The Remainder Stack Count that couldn't be added (if any)
	 * @return	ItemData:		ItemInstanceData
	 */
	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	void AddItem(int32& StackRemainder, FItemInstanceData& OutItemData, UPARAM(meta = (Categories = "Item")) FGameplayTag ItemTag = FGameplayTag(), int32 StackCount = 1);

	UFUNCTION(Server, Reliable, WithValidation)
    void Server_AddItem(FGameplayTag ItemTag, int32 StackCount);
	bool Server_AddItem_Validate(FGameplayTag ItemTag, int32 StackCount) { return true; };

	/**
	 * Drop Item from Inventory
	 *
	 * @param ItemTag:			The Item Tag to drop
	 * @param StackCount:		The Stack Count to drop
	 *
	 * @return	ItemData:		ItemInstanceData
	 */
	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	void DropItem(FItemInstanceData& OutItemData, UPARAM(meta = (Categories = "Item")) FGameplayTag ItemTag = FGameplayTag(), int32 StackCount = 1);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_DropItem(FGameplayTag ItemTag, int32 StackCount);
	bool Server_DropItem_Validate(FGameplayTag ItemTag, int32 StackCount) { return true; };

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryComponent", meta = (DisplayName="Drop Item Instance"))
	void Server_DropItemInstance(UOrionInventoryItemInstance* ItemInstance, int32 StackCount);

	void SpawnDroppedItem(TSoftClassPtr<class AActor> PickupActorClass, FGameplayTag ItemTag, int32 StackCount);

	/**
	 * Consume item from Inventory
	 *
	 * @param ItemTag:			The Item Tag to consume
	 * @param StackCount:		The Stack Count to consume
	 *
	 * @return	ItemData:		ItemInstanceData
	 */
	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	void ConsumeItem(FItemInstanceData& OutItemData, UPARAM(meta = (Categories = "Item")) FGameplayTag ItemTag = FGameplayTag(), int32 StackCount = 1);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ConsumeItem(FGameplayTag ItemTag, int32 StackCount);
	bool Server_ConsumeItem_Validate(FGameplayTag ItemTag, int32 StackCount) { return true; };

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "InventoryComponent", meta = (DisplayName = "Consume Item Instance"))
	void Server_ConsumeItemInstance(UOrionInventoryItemInstance* ItemInstance, int32 StackCount);

	/**
	 * Set inventory max weight, 
	 *
	 * @param  InNewMaxWeight		New max weight to set
	 */
	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	void SetInventoryMaxWeight(float InNewMaxWeight);

	/**
	 * Set inventory Grid dimension,
	 *
	 */
	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	void SetInventoryGridDimension(FIntPoint InNewDimension);

	UFUNCTION(BlueprintCallable, Category = "InventoryComponent", BlueprintPure = false)
	TArray<UOrionInventoryItemInstance*> GetAllItems() const;


	UFUNCTION(BlueprintCallable, Category = "InventoryComponent", BlueprintPure = false)
	void SetEntries(TArray<UOrionInventoryItemInstance*> InItemList);

	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	void AssignItemIndex(UOrionInventoryItemInstance* ItemInstance, int32 TileIndex);

	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	UOrionInventoryNode_ItemDefinition* FindItemDefinition(UPARAM(meta = (Categories = "Item")) FGameplayTag ItemTag);

	/**
	 * Get Item Instance
	 *
	 * @param ItemTag:				The Item Tag
	 *
	 * @return	ItemInstance		Item Instance from the inventory list
	 */
	UFUNCTION(BlueprintPure, Category = "InventoryComponent")
	UOrionInventoryItemInstance* GetItemInstanceByTag(UPARAM(meta = (Categories = "Item")) FGameplayTag ItemTag);

	/**
	 * Get Item Instance data
	 *
	 * @param ItemTag:				The Item Tag
	 *
	 * @return	bSuccess:			return true if item found in Inventory List
	 * @return	ItemData:			ItemInstanceData
	 */
	UFUNCTION(BlueprintPure, Category = "InventoryComponent")
	bool GetItemData(FItemInstanceData& OutItemData, UPARAM(meta = (Categories = "Item")) FGameplayTag ItemTag);


	bool IsValidSlot(class UOrionInventoryNode_ItemDefinition* ItemDef, int32 SlotIndex);

	/**
	 * Returns the Component Owner Role
	 */
	UFUNCTION(BlueprintPure, Category = "InventoryComponent")
	FORCEINLINE ENetRole GetRole() const
	{
		return IsValid(GetOwner()) ? GetOwner()->GetLocalRole() : ENetRole::ROLE_None;
	};
	///
	UFUNCTION(BlueprintPure, Category = "InventoryComponent")
	FORCEINLINE bool IsValidItemTag(FGameplayTag ItemTag) const
	{
		return ItemTag.MatchesTag(TAG_Item) && ItemTag != FGameplayTag::EmptyTag;
	};

	/// <summary>
	/// Inspect Item
	/// </summary>
	/// <param name="ItemTag"></param>
	/// <param name="ActorToInspect"></param>
	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	void InspectItem(UPARAM(meta = (Categories = "Item")) FGameplayTag ItemTag = FGameplayTag());

	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	void InspectItemInstance(UOrionInventoryItemInstance* ItemInstance = nullptr);


	/////////////////////////////////////////////////////////////////////////////////////
	//EQUIPMENT
	/////////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintPure, Category = "InventoryComponent")
	bool GetEquippedItemFromSlot(UOrionInventoryItemInstance*& OutItemInstance, UOrionEquipmentInstance*& OutEquipInstance, UPARAM(meta = (Categories = "EquipSlot")) FGameplayTag InEquipSlot);

	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	UOrionEquipmentInstance* EquipItemFromInstance(UOrionInventoryItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	UOrionEquipmentInstance* EquipItemFromTag(UPARAM(meta = (Categories = "Item")) FGameplayTag ItemTag);


	UOrionEquipmentInstance* EquipItem(FOrionEquipmentDefinition EquipmentDefinition);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_EquipItem(FOrionEquipmentDefinition EquipmentDefinition);
	bool Server_EquipItem_Validate(FOrionEquipmentDefinition EquipmentDefinition) { return true; };


	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	void UnequipItemFromSlot(UPARAM(meta = (Categories = "EquipSlot")) FGameplayTag InEquipSlot);

	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	void UnequipItem(UOrionEquipmentInstance* EquipmentInstance);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_UnequipItem(UOrionEquipmentInstance* EquipmentInstance);
	bool Server_UnequipItem_Validate(UOrionEquipmentInstance* EquipmentInstance) { return true; };


	UFUNCTION(BlueprintCallable, Category = "InventoryComponent", BlueprintPure = false)
	TArray<UOrionEquipmentInstance*> GetAllEquippedItems() const;


	/////////////////////////////////////////////////////////////////////////////////////
	//Loot Item
	/////////////////////////////////////////////////////////////////////////////////////

	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	void LootItemFromActor(AActor* InActor);

	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	void LootItem(class UOrionInteractionTargetComponent_LootItem* InLootComponent);

	/////////////////////////////////////////////////////////////////////////////////////
	//Currency 
	/////////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	bool AddCurrency(UPARAM(meta = (Categories = "Currency"))FGameplayTag InCurrencyTag, int32 Value, FOrionCurrencyData& OutCurrency);

	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	bool GetCurrency(UPARAM(meta = (Categories = "Currency"))FGameplayTag InCurrencyTag, FOrionCurrencyData& OutCurrency);


public:
	UFUNCTION()
	FOrionInventorySaveData GetSaveGameData();

	UFUNCTION()
	void InitializeFromSaveGame(const FOrionInventorySaveData& InventorySaveData);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_InitializeFromSaveGame(const FOrionInventorySaveData& InventorySaveData);
    bool Server_InitializeFromSaveGame_Validate(const FOrionInventorySaveData& InventorySaveData) { return true; }

	bool bInitializingSaveGame;

public:
	UFUNCTION(BlueprintPure, Category = "InventoryComponent")
	virtual APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintPure, Category = "InventoryComponent")
	virtual APlayerController* GetOwningController() const;



public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


// ================================================================================================ //
// CONSOLE COMMANDS
// ================================================================================================ //
public:
	UFUNCTION(Exec)
	void execLogInventory();

	UFUNCTION(Exec)
	void execAddItem(const TArray<FString>& ItemTag);

	UFUNCTION(Exec)
	void execDropItem(const TArray<FString>& ItemTag);
};
