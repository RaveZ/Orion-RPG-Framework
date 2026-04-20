#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "KeyParams.h"
#include "DialogGenerateCameraSettings.generated.h"

UCLASS()
class DIALOG_SYSTEM_EDITOR_API UDialogGenerateCameraSettings : public UObject
{
	GENERATED_BODY()

public:
	UDialogGenerateCameraSettings();

	void Initialize();

public:
	UPROPERTY(EditAnywhere, Category = "Sequence")
	EMovieSceneKeyInterpolation KeyInterpolation = EMovieSceneKeyInterpolation::Constant;

	/** Randomize camera angle for each preset
	* This will provide random variation for each shot
	*/
	UPROPERTY(EditAnywhere, Category = "Camera")
	bool bRandomizeCameraAngles;

	UPROPERTY(EditAnywhere, Category = "Camera")
	bool bRandomizeCameraPresets;

	UPROPERTY(Transient, EditAnywhere, Instanced, Category = "Camera", meta = (EditCondition = "bRandomizeCameraPresets == false", HideEditConditionToggle, EditConditionHides))
	TObjectPtr<class UDialogSequenceShot> ShotToUse;

	UPROPERTY(Transient, EditAnywhere, Instanced, Category = "Camera", meta = (EditCondition = "bRandomizeCameraPresets == true", HideEditConditionToggle, EditConditionHides))
	TArray<TObjectPtr<class UDialogSequenceShot>> DefaultShots;



};