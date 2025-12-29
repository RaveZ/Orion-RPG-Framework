// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Inventory/InventoryData.h"
#include "Containers/Map.h"
#include "OrionInventoryGraph.generated.h"

class UOrionInventoryComponent;
class UOrionInventoryItemFragment;
class UOrionInventoryNode;
class APlayerController;
class APawn;

UCLASS(Blueprintable, BlueprintType)
class ORIONRPG_API UOrionInventoryGraph : public UObject
{
	GENERATED_BODY()
public:
	UOrionInventoryGraph();
	virtual ~UOrionInventoryGraph();

	/** AssetID */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "OrionInventory")
		FName ID;

	UPROPERTY(EditAnywhere, Category = "OrionInventory")
		EOrionInventoryType InventoryType;

	/**Max Weight for this inventory*/
	UPROPERTY(EditAnywhere, meta = (Units = "Kilograms", UIMin = "0", EditCondition = "InventoryType == EOrionInventoryType::E_WeightBased", HideEditConditionToggle, EditConditionHides), Category = "OrionInventory")
		float DefaultMaxInventoryWeight;

	/**
	* Default grid dimension for this inventory
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "OrionInventory", meta = (ClampMin = "1", ClampMax = "100", UIMin = "1", UIMax = "100"))
		FIntPoint DefaultGridDimension;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "OrionInventory", meta = (ClampMin = "50", ClampMax = "100", UIMin = "50", UIMax = "100"))
		float GridSize;

	/// <summary>
	/// Equipment default slots
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "OrionInventory", meta=(Categories="Equipment.Slot"))
		TArray<FGameplayTag> EquipmentDefaultSlots;

	/**Default drop item fragment for each item for this graph*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "OrionInventory", Instanced)
		TObjectPtr<class UOrionInventoryFragment_DropItem> DefaultDropItemFragment;

	UPROPERTY(BlueprintReadWrite, Category = "OrionInventory")
		TObjectPtr<APlayerController> OwningController;

	UPROPERTY(BlueprintReadWrite, Category = "OrionInventory")
		TObjectPtr < UOrionInventoryComponent> OrionInventoryComponent;

	UPROPERTY(BlueprintReadOnly, Category = "OrionInventory")
		TArray< TObjectPtr<UOrionInventoryNode>> RootNodes;

	UPROPERTY(BlueprintReadOnly, Category = "OrionInventory")
		TArray< TObjectPtr<UOrionInventoryNode>> AllNodes;

	UPROPERTY(BlueprintReadOnly, Category = "OrionInventory")
		TMap<FName, TObjectPtr<UOrionInventoryNode>> NodeMap;

	UPROPERTY(BlueprintReadOnly, Category = "OrionInventory")
		TMap<FGameplayTag, TObjectPtr<class UOrionInventoryNode_ItemDefinition>> ItemDefinitions;

	bool bAssetInitialized;

public:
	virtual bool Initialize(UOrionInventoryComponent* InitializingComp);
	virtual void Deinitialize();
	virtual UWorld* GetWorld() const override;
	virtual void PostInitProperties() override;

	UFUNCTION(BlueprintCallable, Category = "OrionInventory")
	void BeginNode(UOrionInventoryNode* InNode, bool bLaunchEventOnLoad = false); 

	void ClearGraph();

	void InitalizeAsset();
	
	UFUNCTION(BlueprintCallable, Category = "OrionInventory")
	APlayerController* GetOwningController();

	UFUNCTION(BlueprintCallable, Category = "OrionInventory")
	APawn* GetOwningPawn();

	UFUNCTION(BlueprintCallable, Category = "OrionInventory")
	UOrionInventoryComponent* GetOrionInventoryComponent();

#if WITH_EDITORONLY_DATA
	/** Set of OrionInventory-graph pages */
	UPROPERTY()
	TArray<TObjectPtr<UEdGraph>> InventoryGraphPages;

	/** Set of documents that were being edited in this blueprint, so we can open them right away */
	UPROPERTY()
	TArray<struct FEditedDocumentInfo> LastEditedDocuments;

	/** Whether or not this blueprint is newly created, and hasn't been opened in an editor yet */
	UPROPERTY(/*transient*/ BlueprintReadOnly, Category = "OrionInventory")
	bool bIsNewlyCreated = true;


	/** Get all graphs in this OrionInventoryEditor */
	void GetAllGraphs(TArray<UEdGraph*>& Graphs) const;
#endif

};


