// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Containers/Map.h"
#include "DialogData.h"
#include "DialogDefinition.h"
#include "Camera/PlayerCameraManager.h"
#include "DialogBuilderGraph.generated.h"

class UDialogComponent;
class UDialogDefinition;
class UDialogSequenceSlot;
class UDialogBuilderNode;
class UDialogBuilderEdge;
class AController;
class APawn;
class UDialogBuilderEdGraph;
class UDialogBuilderNode_DialogLine;



UCLASS(Blueprintable, BlueprintType, AutoExpandCategories = "Configuration|Player", AutoExpandCategories = "Configuration|Dialog")
class ORIONRPG_API UDialogBuilderGraph : public UObject
{
	GENERATED_BODY()
public:
	UDialogBuilderGraph();
	virtual ~UDialogBuilderGraph();

	/** AssetID */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Dialog")
		FName ID;

	/** Your Player Name*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Configuration|Player")
		FText PlayerName;

	UPROPERTY(EditAnywhere, Category = "Configuration|Player", meta = (InlineEditConditionToggle))
		bool bOverrideTransform;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration|Player", meta = (EditCondition = "bOverrideTransform"))
		FTransform PlayerTransformOverride;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Configuration|Player", meta = (NoResetToDefault))
		TObjectPtr<class UDialogCameraShot> DefaultPlayerShot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Configuration|Player", meta = (NoResetToDefault))
		TObjectPtr<class UDialogCameraShot> DefaultSelectingChoiceShot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", Category = "Configuration|Player", DisplayThumbnail = "true", AllowedClasses = "/Script/Engine.Texture,/Script/Engine.MaterialInterface,/Script/Engine.SlateTextureAtlasInterface", DisallowedClasses = "/Script/MediaAssets.MediaTexture"))
		TObjectPtr<UObject> DefaultPlayerImage;

	/*
	* Rotate all participant to whoever is currently speaking
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration|Dialog")
		bool bAutoRotateParticipant;

	/**Auto advance dialog automatically when current dialog line has finished*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration|Dialog")
		bool bAutoAdvanceDialogLine;

	/**Lock player movement when dialog in progress.
	* If set to false, auto advance dialog and won't show mouse cursor.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration|Dialog")
		bool bLockPlayerMovement;

	/**
	* Restore all actor transform after the dialog ends
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration|Dialog")
		bool bRestoreState;

	/**
	* Can this dialog be interrupted by another dialog (if another dialog is triggered while this one is active)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration|Dialog", meta = (DisplayName = "Can Interrupt this Dialog"))
		bool bCanInterruptDialog;

	/*
	* By default dialog camera will aim at the bone set  in this variable
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration|Dialog")
		FName DefaultBoneToTrack;


	/**Camera*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		TSubclassOf<class UCameraShakeBase> DialogCameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		bool bBlendCameraOnDialogEnd;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		bool bShouldBlendCameraOnDialogEnd = false;

	/**time taken to blend*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (EditCondition = "bBlendCameraOnDialogEnd == true", HideEditConditionToggle, EditConditionHides))
		float BlendTime;

	/** Cubic, Linear etc functions for blending*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (EditCondition = "bBlendCameraOnDialogEnd == true", HideEditConditionToggle, EditConditionHides))
		TEnumAsByte<EViewTargetBlendFunction> BlendFunc;

	/**Exponent, used by certain blend functions to control the shape of the curve. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (EditCondition = "bBlendCameraOnDialogEnd == true", HideEditConditionToggle, EditConditionHides))
		float BlendExp;

	/**Fade camera before starting dialog*/
	UPROPERTY(BlueprintReadOnly, Category = "Camera")
		bool bFadeCameraOnDialogBegin;

	UPROPERTY(BlueprintReadOnly, Category = "Audio")
		class USoundAttenuation* DialogSoundAttenuation;
	


	UPROPERTY(BlueprintReadWrite, Category = "DialogBuilderGraph")
		TObjectPtr<APlayerController> OwningController;

	UPROPERTY(BlueprintReadWrite, Category = "DialogBuilderGraph")
		TObjectPtr < UDialogComponent> DialogComponent;

	/** List of All Visited Node IDs in Sequence*/
	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
		TArray<FName> VisitedNodeIDs;

	/*------------------------Dialog Sequence------------------------*/
	UPROPERTY(BlueprintReadOnly, Category = "Detail")
		TObjectPtr<class UDialogSequence> CurrentDialogSequence;

	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
		TArray<TObjectPtr<class UDialogParticipant>> ParticipantDefinitions;

	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
		TObjectPtr<class UDialogPlayerParticipant> PlayerParticipantDefinition;

	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
		TArray<TObjectPtr<class UDialogProp>> PropDefinitions;

	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
		TArray<TObjectPtr<class UDialogCamera>> CameraDefinitions;

	/**List of dialog set used in this graph*/
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = "Detail")
		TArray<TObjectPtr<class UDialogStage>> DialogStages;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = "Detail")
		TObjectPtr<class UDialogStage> CurrentDialogStage;

	UPROPERTY(Transient)
		TMap<UDialogSequenceSlot*, TWeakObjectPtr<AActor>> DialogSlotActors;

	/*------------------------End Dialog Sequence------------------------*/

	/** List of all participant Info in this dialog*/
	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
		TMap<FGameplayTag, FParticipantInfo> ParticipantInfoMap;

	/**Cached actor reference of this dialog, each actor has their own unique tags*/
	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
		TMap<FName, TObjectPtr<AActor>> CachedActorMap;

		/**List of spawned actor in dialog, this specific actors will be destroyed after dialog ended*/
	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
		TArray<TObjectPtr<AActor>> SpawnedDialogActors;

	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
		TObjectPtr<AActor> SequencePivot;

	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
		TObjectPtr<UDialogBuilderNode> CurrentNode;

	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
		TObjectPtr<class UDialogBuilderNode_DialogLine> CurrentLine;

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
		TObjectPtr<class ALevelSequenceActor> DialogSequenceActor;

	//The cinecam spawned in by the sequence 
	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
		TWeakObjectPtr<class ACineCameraActor> Cinecam;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
		TWeakObjectPtr<class AActor> CachedLastSpeaker;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
		TWeakObjectPtr<class UDialogCameraShot> CachedLastShot;
		
	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
		class UAudioComponent* DialogAudio;

protected:
	UFUNCTION()
	void HandleDialogSequenceFinished();

	FTimerHandle DialogLineTimer;
public:
	virtual bool Initialize(UDialogComponent* InitializingComp);
	virtual void Deinitialize();
	virtual UWorld* GetWorld() const override;

	void StartFromRoot();


	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void BeginNode(UDialogBuilderNode* InNode, bool bLaunchEventOnLoad = false);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	virtual void BeginDialogLine(class UDialogBuilderNode_DialogLine* InDialogLine);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	virtual void ApplyRotationSetting(class UDialogBuilderNode_DialogLine* InDialogLine, AActor* Speaker, AActor* Listener);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	virtual void DetermineDialogShot(class UDialogBuilderNode_DialogLine* InDialogLine, AActor* Speaker, AActor* Listener);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	virtual void BeginChoiceSelection(class UDialogBuilderNode_PlayerChoice* InPlayerChoice);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	virtual void BeginDialogSequence(class UDialogBuilderNode_DialogSequence* InDialogSequence);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	virtual void PlayDialogShot(class UDialogCameraShot* InDialogShot, AActor* InSpeaker);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	virtual void PlaySequence(class ULevelSequence* InSequence, FMovieSceneSequencePlaybackSettings InPlaybackSetting);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	virtual void JumpToNextDialogSection();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	FText GetParticipantName(FGameplayTag ParticipantTag) const;

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

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsChoiceSelectionActive() const;

	UFUNCTION(BlueprintCallable, Category = "Dialog")
		virtual void AdvanceDialogLine();

	UFUNCTION(BlueprintCallable, Category = "Dialog")
		virtual void StopCurrentDialogLine();

	UFUNCTION(BlueprintCallable, Category = "Dialog")
		virtual void EndDialog();

	UFUNCTION(BlueprintCallable, Category = "Dialog")
		AActor* GetDialogDefinitionActor(UDialogDefinition* InDialogDefinition);

	UFUNCTION(BlueprintCallable, Category = "Dialog")
	UDialogDefinition* GetDialogDefinitionByTag(FGameplayTag InTag);

	UFUNCTION()
	virtual void DetermineAdvanceDialogRule(class UDialogBuilderNode* InDialogNode);

	UFUNCTION()
	virtual FVector GetSpeakerBoneLocation(class AActor* Actor, bool bOverrideTrackedBone = false, FName BoneOverride = NAME_None) const;

	/**Ensure player definition exist for this graph, create a new one if null*/
	UFUNCTION()
	void EnsurePlayerDefinition();

protected:
	virtual AActor* RetrieveParticipant(FParticipantInfo& Info);
	virtual AActor* RetrieveActorFromSlot(class UDialogSequenceSlot* InSlot);

	void InitializeDialogActors();
	void SetDialogStage(class UDialogStage* InDialogStage);	
	void DestroyDialogSequenceActor();

	/** Cached actor original transforms, used to restore actor transforms after dialog ends */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Dialog")
	TMap<FName, FTransform> CachedActorOriginalTransforms;

protected:
	void CacheOriginalActorTransform(const FName& ActorId, AActor* Actor);
	void RestoreCachedActorTransforms();

#if WITH_EDITORONLY_DATA
public:
	UPROPERTY(Transient)
	TObjectPtr<class UDialogBuilderNode_DialogSequence> CurrentEditingSequenceNode;
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

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	template<typename TNodeType>
	void GetNodesOfType(TArray<TNodeType*>& OutNodes) const
	{
		OutNodes.Reset();

		for (UDialogBuilderNode* Node : AllNodes)
		{
			if (TNodeType* TypedNode = Cast<TNodeType>(Node))
			{
				OutNodes.Add(TypedNode);
			}
		}
	}

	void GetNodesOfType(TSubclassOf<UDialogBuilderNode> NodeType, TArray<UDialogBuilderNode*>& Nodes) const;
};




