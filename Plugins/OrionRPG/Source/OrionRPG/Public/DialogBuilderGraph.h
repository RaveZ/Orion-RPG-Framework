// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Containers/Map.h"
#include "DialogBuilderGraph.generated.h"

class UDialogComponent;
class UDialogBuilderNode;
class UDialogBuilderEdge;
class AController;
class APawn;
class UDialogBuilderEdGraph;
class UDialogBuilderNode_DialogLine;

UCLASS(Blueprintable, BlueprintType)
class ORIONRPG_API UDialogBuilderGraph : public UObject
{
	GENERATED_BODY()
public:
	UDialogBuilderGraph();
	virtual ~UDialogBuilderGraph();

	/** AssetID */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "DialogBuilderGraph")
		FName ID;

	UPROPERTY(BlueprintReadWrite, Category = "DialogBuilderGraph")
		TObjectPtr<AController> OwningController;

	UPROPERTY(BlueprintReadWrite, Category = "DialogBuilderGraph")
		TObjectPtr < UDialogComponent> DialogComponent;

	/** List of All Visited Node IDs in Sequence*/
	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
		TArray<FName> VisitedNodeIDs;

	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
		TObjectPtr<UDialogBuilderNode> CurrentNode;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
		TArray< TObjectPtr<UDialogBuilderNode>> RootNodes;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
		TArray< TObjectPtr<UDialogBuilderNode>> AllNodes;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
		TMap<FName, TObjectPtr<UDialogBuilderNode>> NodeMap;

public:
	virtual bool Initialize(UDialogComponent* InitializingComp);
	virtual void Deinitialize();

	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void BeginNode(UDialogBuilderNode* InNode, bool bLaunchEventOnLoad = false);
	
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	int GetLevelNum() const;

	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void GetNodesByLevel(int Level, TArray<UDialogBuilderNode*>& Nodes);

	void ClearGraph();
	
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	AController* GetOwningController();

	UFUNCTION(BlueprintCallable, Category = "Dialog")
	APawn* GetOwningPawn();

	UFUNCTION(BlueprintCallable, Category = "Dialog")
	UDialogComponent* GetDialogComponent();

public:
	// Collect all unique shared participant and group them by their name.
	/*UPROPERTY()
	TMultiMap<FString, TObjectPtr<UDialogBuilderNode_DialogLine>> SharedParticipants;*/

#if WITH_EDITORONLY_DATA
	/** Set of dialog-graph pages */
	UPROPERTY()
	TArray<TObjectPtr<UEdGraph>> DialogGraphPages;

	/** Set of documents that were being edited in this blueprint, so we can open them right away */
	UPROPERTY()
	TArray<struct FEditedDocumentInfo> LastEditedDocuments;

	/** Whether or not this blueprint is newly created, and hasn't been opened in an editor yet */
	UPROPERTY(/*transient*/ BlueprintReadOnly, Category = "DialogBuilderGraph")
	bool bIsNewlyCreated = true;


	/** Get all graphs in this DialogEditor */
	void GetAllGraphs(TArray<UEdGraph*>& Graphs) const;
#endif

};

