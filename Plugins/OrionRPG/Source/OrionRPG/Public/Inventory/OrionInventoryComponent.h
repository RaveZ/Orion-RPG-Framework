// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OrionInventoryComponent.generated.h"

class UOrionInventorySaveObject;
class UOrionInventoryGraph;
class USaveGame;
class APlayerController;

//OrionInventory Delegate

UCLASS(meta=(BlueprintSpawnableComponent), Blueprintable, HideDropdown, ClassGroup = "OrionRPG", DisplayName = "Orion Inventory Component")
class ORIONRPG_API UOrionInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOrionInventoryComponent();


	/**Inventory database*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orion")
	TObjectPtr<UOrionInventoryGraph> InventoryData;

	//We cache the OwningController, we won't cache pawn as this might change
	UPROPERTY(BlueprintReadOnly, Category = "OrionInventoryComponent")
		TObjectPtr<APlayerController> OwningController;

	UPROPERTY(BlueprintReadWrite, Category = "OrionInventoryComponent")
		TMap<FName, TObjectPtr<UOrionInventoryGraph>> OrionInventoryMap;
public:
	
	UFUNCTION(BlueprintCallable, Category = "OrionInventoryComponent")
		bool BeginOrionInventory(UOrionInventoryGraph* OrionInventoryAsset);

	UFUNCTION()
		virtual class UOrionInventoryGraph* MakeOrionInventoryGraphInstance(UOrionInventoryGraph* OrionInventoryGraphTemplate);

public:
	UFUNCTION(BlueprintPure, Category = "OrionInventoryComponent")
	virtual APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintPure, Category = "OrionInventoryComponent")
	virtual APlayerController* GetOwningController() const;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
