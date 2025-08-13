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
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Dialog_Participant_Player);

UENUM(BlueprintType)
enum class ESelectionTimeLimit : uint8
{
	E_NoTimeLimit		UMETA(DisplayName = "No Time Limit"),
	E_HasTimeLimit		UMETA(DisplayName = "Has Time Limit")
};

UENUM(BlueprintType)
enum class EParticipantSetup : uint8
{
	/**
	* Participant is linked to the world.
	* To link the participant, all you need to do is to add the actor tags the same as the participant tag.
	* By default, all you need to do is just add the Interaction target component and fill the tag.
	* By doing this, the tag will be automatically assigned to the actor tags.
	*/
	E_LinkedToWorld		UMETA(DisplayName = "Linked To World"),
	/**
	* Spawn the participant instead.
	* When dialog ended, destroy the participant.
	*/
	E_SpawnParticipant	UMETA(DisplayName = "Spawn Participant")
};

UENUM(BlueprintType)
enum class EDialogCameraMode : uint8
{
	/**no generated shot in this line, will use the last shot instead*/
	E_Disabled					UMETA(DisplayName = "Disabled"),
	/**Shot will be generated in this line. If override shot is valid, will use that shot instead.*/
	E_GeneratedCameraShot		UMETA(DisplayName = "Generated Camera Shot"),
	/**Shot will be generated using sequence instead.*/
	E_Sequence					UMETA(DisplayName = "Sequence")
};

USTRUCT(BlueprintType)
struct FDialogLineData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MultiLine = true, NoResetToDefault, DisplayPriority = -1))
	FText Line;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USoundBase* DialogSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Animation"))
	class UAnimMontage* DialogMontage;

	/**Participant image override, will use this instead of default participant image.
	* Useful if you want to have different expression for your participant.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", DisplayThumbnail = "true", AllowedClasses = "/Script/Engine.Texture,/Script/Engine.MaterialInterface,/Script/Engine.SlateTextureAtlasInterface", DisallowedClasses = "/Script/MediaAssets.MediaTexture"))
	TObjectPtr<UObject> ParticipantImageOverride;


	/**
	* Select ypur shot method to override.
	* instead of generated shot, you can play sequence instead.
	**/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shot Override")
	EDialogCameraMode DialogCameraMode = EDialogCameraMode::E_GeneratedCameraShot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shot Override", meta = (EditCondition = "DialogCameraMode == EDialogCameraMode::E_Sequence", HideEditConditionToggle, EditConditionHides))
	TObjectPtr<class ULevelSequence> SequenceToPlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot Override", meta = (EditCondition = "DialogCameraMode == EDialogCameraMode::E_Sequence", HideEditConditionToggle, EditConditionHides))
	FMovieSceneSequencePlaybackSettings PlaybackSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Shot Override", meta = (EditCondition = "DialogCameraMode == EDialogCameraMode::E_GeneratedCameraShot", HideEditConditionToggle, EditConditionHides))
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

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog", meta = (Categories = "Dialog.Participant"))
	FGameplayTag ParticipantTag;

	/** Your Participant Name*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Dialog")
	FText ParticipantName;


	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Dialog")
	EParticipantSetup ParticipantSetup = EParticipantSetup::E_LinkedToWorld;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog",  meta = (EditCondition = "ParticipantSetup == EParticipantSetup::E_SpawnParticipant", HideEditConditionToggle, EditConditionHides))
	TSubclassOf<class AActor> ParticipantToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog", meta = (EditCondition = "ParticipantSetup == EParticipantSetup::E_SpawnParticipant", HideEditConditionToggle, EditConditionHides))
	FTransform ParticipantTransform;

	/**Participant image to be shown while participant line in progress. 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog", meta = (AllowPrivateAccess = "true", DisplayThumbnail = "true", DisplayName = "Participant Image", AllowedClasses = "/Script/Engine.Texture,/Script/Engine.MaterialInterface,/Script/Engine.SlateTextureAtlasInterface", DisallowedClasses = "/Script/MediaAssets.MediaTexture"))
	TObjectPtr<UObject> ParticipantImage;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = "Dialog", meta = (NoResetToDefault))
	TObjectPtr<UDialogCameraShot> DefaultShot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog", meta = (NoResetToDefault))
	FLinearColor NodeColor;

	/**participant initial transforms, used to get participant back in this transfrom when dialog ended*/
	UPROPERTY(BlueprintReadWrite, Category = "Dialog")
	FTransform InitialTransform;

};

