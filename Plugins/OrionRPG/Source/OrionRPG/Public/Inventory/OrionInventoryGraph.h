// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Containers/Map.h"
#include "OrionInventoryGraph.generated.h"

class UOrionInventoryComponent;
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
		TMap<FGameplayTag, TObjectPtr<class UOrionInventoryNode_Item>> ItemDefinitionMap;

public:
	virtual bool Initialize(UOrionInventoryComponent* InitializingComp);
	virtual void Deinitialize();
	virtual UWorld* GetWorld() const override;

	UFUNCTION(BlueprintCallable, Category = "OrionInventory")
	void BeginNode(UOrionInventoryNode* InNode, bool bLaunchEventOnLoad = false); 

	void ClearGraph();
	
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
	UPROPERTY(/*transient*/ BlueprintReadOnly, Category = "OrionInventoryGraph")
	bool bIsNewlyCreated = true;


	/** Get all graphs in this OrionInventoryEditor */
	void GetAllGraphs(TArray<UEdGraph*>& Graphs) const;
#endif

};


