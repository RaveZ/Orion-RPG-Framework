// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Inventory/InventoryData.h"
#include "Components/ActorComponent.h"
#include "OrionSaveGame.h"
#include "OrionInventoryComponent.generated.h"

class UOrionInventoryGraph;
class UOrionInventoryNode_ItemDefinition;
class UOrionInventoryItemInstance;
class APlayerController;
struct FNetDeltaSerializeInfo;
struct FReplicationFlags;
struct FFrame;

struct FInventoryItem;

//Inventory Delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInventorySignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryItemSignature, UOrionInventoryItemInstance*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryNoRoomForItem, UOrionInventoryNode_ItemDefinition*, InItemDefinition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInventoryExamineItem, UOrionInventoryItemInstance*, Item, TSoftClassPtr<AActor>, ActorToExamine);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInventoryGridDimensionChanged, FIntPoint, InOldDimension, FIntPoint, InNewDimension);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FInventoryItemStackChange, UOrionInventoryItemInstance*, Item, int32, CurrentStackCount, int32, MaxStackCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryMaxWeightChanged, float, InMaxWeight);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryEncumberedStateChanged, bool, bEncumbered);

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
		TObjectPtr<AActor> ItemViewerActor;

	
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
		FInventoryItemSignature OnItemAdded;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
		FInventoryItemSignature OnItemRemoved;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
		FInventoryItemSignature OnItemConsumed;


	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
	FInventoryItemStackChange OnItemStackChanged;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
		FInventoryMaxWeightChanged OnMaxWeightChanged;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
	FInventoryGridDimensionChanged OnGridDimensionChanged;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
	FInventoryNoRoomForItem OnNoRoomForItem;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
		FInventoryEncumberedStateChanged OnEncumberedStateChanged;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
		FInventoryExamineItem OnExamineItem;

public:
	UFUNCTION()
	virtual void GridDimensionChanged(FIntPoint OldDimension, FIntPoint NewDimension);
	 
private:
	UPROPERTY(Replicated)
	FOrionInventoryList InventoryList;

	/**Max weight for this inventory*/
	float MaxWeight;

	/**Current weight for this inventory*/
	float CurrentWeight;

	FIntPoint CurrentGridDimension;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitializeDelegate();

	//~UObject interface
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	//~End of UObject interface

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

	UFUNCTION(BlueprintPure, Category = "InventoryComponent")
	FORCEINLINE bool IsValidItemTag(FGameplayTag ItemTag) const
	{
		return ItemTag.MatchesTag(TAG_Item) && ItemTag != FGameplayTag::EmptyTag;
	};


	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	void ExamineItem(UPARAM(meta = (Categories = "Item")) FGameplayTag ItemTag = FGameplayTag(), TSoftClassPtr<AActor> ActorToExamine = nullptr);

	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
	void ExamineItemInstance(UOrionInventoryItemInstance* ItemInstance = nullptr, TSoftClassPtr<AActor> ActorToExamine = nullptr);

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
