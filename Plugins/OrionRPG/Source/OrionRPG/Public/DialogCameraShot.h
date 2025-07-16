// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <MovieSceneSequencePlayer.h>
#include <CineCameraSettings.h>
#include "LevelSequencePlayer.h"
#include "DialogCameraShot.generated.h"


UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, AutoExpandCategories = "Detail")
class ORIONRPG_API UDialogCameraShot : public UObject {
	GENERATED_BODY()

public:
	UDialogCameraShot();
	/**Cinecam distance from the track location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail")
	float CameraDistance;

	/** Current focal length of the camera (i.e. controls FoV, zoom) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail|Camera Settings")
	float FocalLengthOverride;

	/** Current aperture, in terms of f-stop (e.g. 2.8 for f/2.8) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail|Camera Settings")
	float ApertureOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail|Camera Settings")
	struct FPlateCropSettings CropSettings;

	/** Controls the filmback of the camera. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail|Camera Settings")
	FCameraFilmbackSettings Filmback;

	/** Controls the camera's lens. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail|Camera Settings")
	FCameraLensSettings LensSettings;

	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle))
	bool bUsePostProcess = false;

	/**Camera post process override*/
	UPROPERTY(Interp, BlueprintReadWrite, Category = "Detail|Camera Settings", meta = (EditCondition = "bUsePostProcess"))
	struct FPostProcessSettings PostProcessSettings;


protected:
	// Allows the Object to get a valid UWorld from it's outer.
	virtual UWorld* GetWorld() const override
	{
		if (HasAllFlags(RF_ClassDefaultObject))
		{
			// If we are a CDO, we must return nullptr instead of calling Outer->GetWorld() to fool UObject::ImplementsGetWorld.
			return nullptr;
		}

		UObject* Outer = GetOuter();

		while (Outer)
		{
			UWorld* World = Outer->GetWorld();
			if (World)
			{
				return World;
			}

			Outer = Outer->GetOuter();
		}

		return nullptr;
	}

};

