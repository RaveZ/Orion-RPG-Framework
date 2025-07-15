// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogBuilderNode.h"
#include "DialogData.h"
#include <MovieSceneSequencePlayer.h>
#include <CineCameraSettings.h>
#include "LevelSequencePlayer.h"
#include "DialogBuilderNode_DialogLine.generated.h"

class ORIONRPG_API IDialogNodeSharedDataHelper
{
public:
	void UpdateSharedData(UDialogBuilderNode_DialogLine* Node, TSharedPtr<INameValidatorInterface> NameValidator);
	void MakeSureGuidExists(UDialogBuilderNode_DialogLine* Node);

protected:
	virtual bool CheckIfNodesShouldShareData(const UDialogBuilderNode_DialogLine* NodeA, const UDialogBuilderNode_DialogLine* NodeB) = 0;
	virtual bool CheckIfHasDataToShare(const UDialogBuilderNode_DialogLine* Node) = 0;
	virtual void ShareData(UDialogBuilderNode_DialogLine* NodeWhoWantsToShare, const UDialogBuilderNode_DialogLine* ShareFrom) = 0;
	virtual FString& AccessShareDataName(UDialogBuilderNode_DialogLine* Node) = 0;
	virtual FGuid& AccessShareDataGuid(UDialogBuilderNode_DialogLine* Node) = 0;
};

//////////////////////////////////////////////////////////////////////////
// FDialogSharedParticipantNodeHelper

class ORIONRPG_API FDialogSharedParticipantNodeHelper : public IDialogNodeSharedDataHelper
{
protected:
	virtual bool CheckIfNodesShouldShareData(const UDialogBuilderNode_DialogLine* NodeA, const UDialogBuilderNode_DialogLine* NodeB) override;
	virtual bool CheckIfHasDataToShare(const UDialogBuilderNode_DialogLine* Node) override;
	virtual void ShareData(UDialogBuilderNode_DialogLine* NodeWhoWantsToShare, const UDialogBuilderNode_DialogLine* ShareFrom) override;
	virtual FString& AccessShareDataName(UDialogBuilderNode_DialogLine* Node) override;
	virtual FGuid& AccessShareDataGuid(UDialogBuilderNode_DialogLine* Node) override;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSharedParticipantSignature);

UCLASS(Blueprintable, BlueprintType)
class ORIONRPG_API UDialogBuilderNode_DialogLine : public UDialogBuilderNode
{
	GENERATED_BODY()

public:
	UDialogBuilderNode_DialogLine();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
		FParticipantInfo ParticipantInfo;

	UPROPERTY()
		FString SharedParticipantName;
	
	UPROPERTY()
		bool bIsSelector = false;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail", meta = (EditCondition = "bIsSelector == true", HideEditConditionToggle, EditConditionHides))
	TObjectPtr<UDialogCameraShot> SelectingOptionShotOverride;
	/**
	* Entering selection mode will have it's time limit setting.
	* Selection will have no time limit by default
	**/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Option Selection", meta = (EditCondition = "bIsSelector == true", HideEditConditionToggle, EditConditionHides))
		ESelectionTimeLimit SelectionTimeLimit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Option Selection", meta = (EditCondition = "bIsSelector == true && SelectionTimeLimit == ESelectionTimeLimit::E_HasTimeLimit", HideEditConditionToggle, EditConditionHides))
		float TimeLimit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail", meta = (ShowOnlyInnerProperties))
		FDialogLineData DialogLineData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail")
		bool bCanSkipDialogLine;

	/** The participant data of this node may be shared */
	UPROPERTY()
		bool bSharedParticipant;

	UPROPERTY()
		FGuid SharedParticipantGuid;

	UPROPERTY()
		int32 SharedParticipantIdx;
	UPROPERTY()
	FSharedParticipantSignature OnSharedParticipantChanged;
	
public:
	virtual void BeginNode() override;

	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostLoad() override;
	virtual void Serialize(FArchive& Ar) override;
	//~ End UObject Interface


public:
#if WITH_EDITOR

	//Participant Sharing
	void MakeParticipantShareable(FString ShareName);
	void UnshareParticipant();
	void UseSharedParticipant(const UDialogBuilderNode_DialogLine* Node);

	void CopyParticipantSettings(const UDialogBuilderNode_DialogLine* SrcNode);
	void PropagateParticipantSettings();
	//Node Setting
	virtual FText GetNodeTitle() const override;
	virtual void SetNodeTitle(const FText& NewTitle);
	virtual FText GetNodeDescription() const override;
#endif
};
