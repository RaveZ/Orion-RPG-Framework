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

	//We cache the OwningController, we won't cache pawn as this might change
	UPROPERTY(BlueprintReadOnly, Category = "OrionInventoryComponent")
		TObjectPtr<APlayerController> OwningController;

	/** Current OrionInventory That is Running, only one OrionInventory should active at a time*/
	UPROPERTY(BlueprintReadWrite, Category = "OrionInventoryComponent")
		TObjectPtr<UOrionInventoryGraph> CurrentActiveOrionInventory;

	UPROPERTY(BlueprintReadWrite, Category = "OrionInventoryComponent")
		TMap<FName, TObjectPtr<UOrionInventoryGraph>> OrionInventoryMap;
public:
	
	UFUNCTION(BlueprintCallable, Category = "OrionInventoryComponent")
		bool BeginOrionInventory(UOrionInventoryGraph* OrionInventoryAsset);

	UFUNCTION()
		virtual class UOrionInventoryGraph* MakeOrionInventoryGraphInstance(UOrionInventoryGraph* OrionInventoryGraphTemplate);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintAuthorityOnly, Category = "Saving")
		void Save(const FString& SaveFileName, int SlotIndex);

	virtual void Save_Implementation(const FString& SaveFileName, int SlotIndex);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintAuthorityOnly, Category = "Saving")
		void Load(const FString& SaveFileName, int SlotIndex);

	virtual void Load_Implementation(const FString& SaveFileName, int SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Saving")
	virtual bool DeleteSave(const FString& SaveName = "OrionInventorySaveData", const int32 Slot = 0);

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
