// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "UObject/NoExportTypes.h"
#include "NativeGameplayTags.h"
#include <MovieSceneSequencePlayer.h>
#include <CineCameraSettings.h>
#include "LevelSequencePlayer.h"
#include "DialogData.generated.h"

class UDialogBuilderNode_DialogLine;
class UDialogCameraShot;

//Gameplay Tags
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Dialog);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Dialog_Participant);

UENUM(BlueprintType)
enum class ESelectionTimeLimit : uint8
{
	E_NoTimeLimit		UMETA(DisplayName = "No Time Limit"),
	E_HasTimeLimit		UMETA(DisplayName = "Has Time Limit")
};

UENUM(BlueprintType)
enum class EDialogCameraMode : uint8
{
	E_GeneratedCameraShot		UMETA(DisplayName = "Generated Camera Shot"),
	E_Sequence					UMETA(DisplayName = "Sequence")
};

USTRUCT(BlueprintType)
struct FDialogLineData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail",  meta = (MultiLine = true, NoResetToDefault))
	FText Line;

	/**
	* Select ypur shot method to override. 
	* instead of generated shot, you play sequence instead. 
	**/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail|Shot Override")
	EDialogCameraMode DialogCameraMode = EDialogCameraMode::E_GeneratedCameraShot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail|Shot Override", meta = (EditCondition = "DialogCameraMode == EDialogCameraMode::E_Sequence", HideEditConditionToggle, EditConditionHides))
	TObjectPtr<class ULevelSequence> SequenceToPlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail|Shot Override", meta = (EditCondition = "DialogCameraMode == EDialogCameraMode::E_Sequence", HideEditConditionToggle, EditConditionHides))
	FMovieSceneSequencePlaybackSettings PlaybackSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Detail|Shot Override", meta = (EditCondition = "DialogCameraMode == EDialogCameraMode::E_GeneratedCameraShot", HideEditConditionToggle, EditConditionHides))
	TObjectPtr<UDialogCameraShot> ShotOverride;

	FDialogLineData()
		: Line(FText::FromString("None"))
	{
		PlaybackSettings.bPauseAtEnd = true;
		PlaybackSettings.LoopCount.Value = 1;
		PlaybackSettings.PlayRate = 1.f;
		PlaybackSettings.StartTime = 0.f;
		PlaybackSettings.bRandomStartTime = false;
		PlaybackSettings.bHidePlayer = false;
		PlaybackSettings.bHideHud = true;
		PlaybackSettings.bDisableCameraCuts = false;
	}
};


USTRUCT(BlueprintType)
struct FParticipantInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog", meta = (Categories = "Dialog.Participant"))
	FGameplayTag ParticipantTag;

	/** Your Participant Name*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Dialog")
	FText ParticipantName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog", meta = (FullyExpand = true))
	TSubclassOf<class AActor> ParticipantClassOverride;

	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle))
	bool bOverrideTransform;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog", meta = (EditCondition = "bOverrideTransform"))
	FTransform ParticipantTransformOverride;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = "Detail")
	TObjectPtr<UDialogCameraShot> DefaultShot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog", meta = (NoResetToDefault))
	FLinearColor NodeColor;
};

