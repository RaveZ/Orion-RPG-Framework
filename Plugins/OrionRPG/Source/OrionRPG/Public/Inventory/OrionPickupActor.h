// Copyright 2025 Ivan Chandra. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Inventory/InventoryData.h"
#include "Interaction/OrionInteractionTargetComponent.h"
#include "OrionPickupActor.generated.h"


UCLASS(Blueprintable, BlueprintType, Abstract)
class ORIONRPG_API AOrionPickupActor : public AActor
{
    GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickupMesh;

	UPROPERTY(EditAnywhere)
	UOrionInteractionTargetComponent_PickupItem* InteractionTargetComp;

public:
	AOrionPickupActor();

	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;

};

/**
 * Interaction target component - pickupable
 */
UCLASS(meta = (BlueprintSpawnableComponent), Blueprintable, HideDropdown, ClassGroup = "OrionRPG", DisplayName = "Orion Interaction Target - Pickup Item", AutoExpandCategories = "Interactable|Pickup")
class UOrionInteractionTargetComponent_PickupItem : public UOrionInteractionTargetComponent
{
	GENERATED_BODY()

public:
	UOrionInteractionTargetComponent_PickupItem();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|Pickup")
	TArray<FOrionPickupDefinition> PickupDefinitions;

public:
	UFUNCTION(BlueprintCallable, Category = "Interactable")
	void InitializePickups();
	

};