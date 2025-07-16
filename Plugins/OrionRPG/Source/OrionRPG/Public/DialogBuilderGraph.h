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
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Detail")
		FName ID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog", meta = (FullyExpand = true))
		TSubclassOf<class AActor> PlayerClassOverride;

	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle))
		bool bOverrideTransform;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog", meta = (EditCondition = "bOverrideTransform"))
		FTransform PlayerTransformOverride;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Configuration")
		TObjectPtr<class UDialogCameraShot> DefaultPlayerShot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Configuration")
		TObjectPtr<class UDialogCameraShot> DefaultSelectingOptionShot;


	/**Camera*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		TSubclassOf<class UCameraShakeBase> DialogCameraShake;
	/*
	* By default dialog camera will aim at the bone set  in this variable
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
		FName DefaultBoneToTrack;

	UPROPERTY(BlueprintReadWrite, Category = "DialogBuilderGraph")
		TObjectPtr<APlayerController> OwningController;

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

	UPROPERTY(BlueprintReadOnly, Category = "Sequence")
		TWeakObjectPtr<class AActor> CachedLastSpeaker;


public:
	virtual bool Initialize(UDialogComponent* InitializingComp);
	virtual void Deinitialize();
	virtual UWorld* GetWorld() const override;

	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void BeginNode(UDialogBuilderNode* InNode, bool bLaunchEventOnLoad = false);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	virtual void BeginDialogShot(class UDialogBuilderNode_DialogLine* InDialogLine);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	virtual void BeginSelectionShot(class UDialogBuilderNode_DialogLine* InDialogLine);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	virtual void PlayDialogShot(class UDialogCameraShot* InDialogShot, AActor* InSpeaker, TArray<AActor*> InListeners, const bool bOverrideShot = false);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	virtual void PlayDialogSequence(const FDialogLineData& InDialogLineData);
	
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	int GetLevelNum() const;

	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void GetNodesByLevel(int Level, TArray<UDialogBuilderNode*>& Nodes);

	void ClearGraph();
	
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	APlayerController* GetOwningController();

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


