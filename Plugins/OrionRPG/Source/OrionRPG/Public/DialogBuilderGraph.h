// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Containers/Map.h"
#include "DialogData.h"
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


	/*
	* By default dialog camera will aim at the bone named "head" - this is the name of the UE4/5 skeletons head bone so will work with most games.
	* If your head bone has a different name, you can input it here - if you need anything more complex simply override the GetSpeakerHeadLocation function
	* and return the location of your avatars head.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
		FName DefaultHeadBoneName;

	UPROPERTY(BlueprintReadWrite, Category = "DialogBuilderGraph")
		TObjectPtr<AController> OwningController;

	UPROPERTY(BlueprintReadWrite, Category = "DialogBuilderGraph")
		TObjectPtr < UDialogComponent> DialogComponent;

	/** List of All Visited Node IDs in Sequence*/
	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
		TArray<FName> VisitedNodeIDs;

	/** List of all participant Info in this dialog*/
	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
		TMap<FGameplayTag, FParticipantInfo> ParticipantInfoMap;

	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
		TMap<FGameplayTag, TObjectPtr<AActor>> CachedParticipants;

	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
		TObjectPtr<UDialogBuilderNode> CurrentNode;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
		TArray< TObjectPtr<UDialogBuilderNode>> RootNodes;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
		TArray< TObjectPtr<UDialogBuilderNode>> AllNodes;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
		TMap<FName, TObjectPtr<UDialogBuilderNode>> NodeMap;

	/**Latest node before entering selection node*/
	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
		TObjectPtr<UDialogBuilderNode> LatestRootSelectionNode;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
		TObjectPtr<class ALevelSequenceActor> DialogSequencer;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
		bool bOptionSelectionActive;

	//The cinecam spawned in by the sequence 
	UPROPERTY(BlueprintReadOnly, Category = "Sequence")
		TWeakObjectPtr<class ACineCameraActor> Cinecam;

	UPROPERTY(BlueprintReadOnly, Category = "Sequence")
		TWeakObjectPtr<class AActor> LookAtActor;


public:
	virtual bool Initialize(UDialogComponent* InitializingComp);
	virtual void Deinitialize();

	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void BeginNode(UDialogBuilderNode* InNode, bool bLaunchEventOnLoad = false);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	virtual void PlayDialogueSequence(class UDialogBuilderNode_DialogLine* InDialogLine);
	
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

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
		virtual void AdvanceDialogLine();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
		virtual void EndDialog();

	UFUNCTION()
	virtual FVector GetSpeakerHeadLocation(class AActor* Actor) const;

protected:
	virtual AActor* RetrieveParticipant(const FParticipantInfo& Info);
	
#if WITH_EDITORONLY_DATA
public:
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


