// Copyright 2025 Ivan Chandra. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OrionSaveGame.h"
#include "Inventory/InventoryData.h"
#include "Interaction/OrionInteractionTargetComponent.h"
#include "OrionItemPickup.generated.h"

/**
 * Orion Lootable Actor
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class ORIONRPG_API AOrionLootable : public AActor, public IOrionSaveGameInterface
{
    GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Lootable")
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Category = "Lootable")
	UOrionInteractionTargetComponent_LootItem* InteractionTargetComp;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Lootable")
	TArray<FOrionPickupDefinition> ContainedItems;

public:
	AOrionLootable();

	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;

	//IOrionSaveGameInterface
	virtual void OnPreSave_Implementation() override;
	virtual void OnSaved_Implementation() override;
	virtual void OnLoaded_Implementation() override;
	//end IOrionSaveGameInterface

};

/**
 * Interaction target component - loot item
 */
UCLASS(meta = (BlueprintSpawnableComponent), Blueprintable, HideDropdown, ClassGroup = "OrionRPG", DisplayName = "Orion Interaction Target - Loot Item", AutoExpandCategories = "Interactable|Loot Item")
class UOrionInteractionTargetComponent_LootItem : public UOrionInteractionTargetComponent
{
	GENERATED_BODY()

public:
	UOrionInteractionTargetComponent_LootItem();

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Interactable|Loot Item")
	TArray<FOrionPickupDefinition> ContainedItems;

	void AddItemToContainer(FOrionPickupDefinition& InPickupDefinition);

	UFUNCTION(BlueprintCallable, Category = "Interactable|Loot Item")
	void TakeAllItems(AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category = "Interactable|Loot Item")
	void TakeItem(FOrionPickupDefinition& OutDef, AActor* Interactor, int32 ItemIndex);


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Interactable|Loot Item")
	bool HasAnyItem() { return !ContainedItems.IsEmpty(); }

	UFUNCTION(BlueprintCallable, Category = "Interactable|Loot Item")
	void EvaluateItemList();

	/*Destroy actor when no contained item left. otherwise, just deactivate the interaction target*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|Loot Item")
	bool bShouldDestroyActor;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnLocalCompleted_Implementation(AActor* Interactor, UOrionInteractionTargetComponent* Component) override;

	virtual bool CanInteract() const override;
};


/**
 * Orion Item Pickup Actor
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class ORIONRPG_API AOrionItemPickup : public AActor, public IOrionSaveGameInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "ItemPickup")
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Category = "ItemPickup")
	UOrionInteractionTargetComponent_PickupItem* InteractionTargetComp;


	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "ItemPickup")
	FOrionPickupDefinition ContainedItem;

public:
	AOrionItemPickup();

	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;

	//IOrionSaveGameInterface
	virtual void OnPreSave_Implementation() override;
	virtual void OnSaved_Implementation() override;
	virtual void OnLoaded_Implementation() override;

	//End IOrionSaveGameInterface

};


/**
 * Interaction target component - pickup item
 */
UCLASS(meta = (BlueprintSpawnableComponent), Blueprintable, HideDropdown, ClassGroup = "OrionRPG", DisplayName = "Orion Interaction Target - Pickup Item", AutoExpandCategories = "Interactable|Pickup Item")
class UOrionInteractionTargetComponent_PickupItem : public UOrionInteractionTargetComponent
{
	GENERATED_BODY()

public:
	UOrionInteractionTargetComponent_PickupItem();

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Interactable|Pickup Item")
	FOrionPickupDefinition ContainedItem;

	/*Destroy actor when no contained item left. otherwise, just deactivate the interaction target*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|Pickup Item")
	bool bShouldDestroyActor;

	void SetItem(FOrionPickupDefinition& InPickupDefinition);
	virtual void OnLocalCompleted_Implementation(AActor* Interactor, UOrionInteractionTargetComponent* Component) override;

	UFUNCTION(BlueprintCallable, Category = "Interactable|Pickup Item")
	void EvaluatePickup();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

