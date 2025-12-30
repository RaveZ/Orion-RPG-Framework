// Copyright 2025 Ivan Chandra. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Inventory/InventoryData.h"
#include "Interaction/OrionInteractionTargetComponent.h"
#include "OrionItemPickup.generated.h"

/**
 * Orion Lootable Actor
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class ORIONRPG_API AOrionLootable : public AActor
{
    GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere)
	UOrionInteractionTargetComponent_LootItem* InteractionTargetComp;

public:
	AOrionLootable();

	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|Loot Item")
	TArray<FOrionPickupDefinition> ItemDefinitions;



};


/**
 * Orion Item Pickup Actor
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class ORIONRPG_API AOrionItemPickup : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere)
	UOrionInteractionTargetComponent_PickupItem* InteractionTargetComp;

public:
	AOrionItemPickup();

	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|Pickup Item")
	FOrionPickupDefinition ItemDefinition;

	

};

