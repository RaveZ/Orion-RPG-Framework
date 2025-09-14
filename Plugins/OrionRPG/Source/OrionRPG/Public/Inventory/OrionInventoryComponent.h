// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OrionInventoryComponent.generated.h"

class UOrionInventoryGraph;
class UOrionInventoryNode_Item;
class APlayerController;

//Inventory Delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryItemSignature, UOrionInventoryNode_Item*, Item);

UCLASS(meta=(BlueprintSpawnableComponent), Blueprintable, HideDropdown, ClassGroup = "OrionRPG", DisplayName = "Orion Inventory Component")
class ORIONRPG_API UOrionInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOrionInventoryComponent();


	/**Inventory database*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryComponent")
	TObjectPtr<UOrionInventoryGraph> InventoryData;

	//We cache the OwningController, we won't cache pawn as this might change
	UPROPERTY(BlueprintReadOnly, Category = "InventoryComponent")
		TObjectPtr<APlayerController> OwningController;

	UPROPERTY(BlueprintReadWrite, Category = "InventoryComponent")
		TMap<FName, TObjectPtr<UOrionInventoryGraph>> OrionInventoryMap;


	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
		FInventoryItemSignature OnItemAdded;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "InventoryComponent")
		FInventoryItemSignature OnItemRemoved;

public:
	
	UFUNCTION(BlueprintCallable, Category = "InventoryComponent")
		bool BeginOrionInventory(UOrionInventoryGraph* OrionInventoryAsset);

	UFUNCTION()
		virtual class UOrionInventoryGraph* MakeOrionInventoryGraphInstance(UOrionInventoryGraph* OrionInventoryGraphTemplate);

public:
	UFUNCTION(BlueprintPure, Category = "InventoryComponent")
	virtual APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintPure, Category = "InventoryComponent")
	virtual APlayerController* GetOwningController() const;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
