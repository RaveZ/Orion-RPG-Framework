// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "DialogCameraShot.h"
#include "DialogBuilderGraph.h"
#include <CineCameraActor.h>
#include <CineCameraComponent.h>

UDialogCameraShot::UDialogCameraShot()
{
	bUsePostProcess = false;
	CameraDistance = 100.f;
	FocalLengthOverride = 35.0f;
	ApertureOverride = 2.8f;
	ConeAngle = FVector2f(30.f, 3.f);

	CropSettings.AspectRatio = 2.39f;

	Filmback.SensorWidth = 23.76f;
	Filmback.SensorHeight = 13.365f;
	Filmback.SensorAspectRatio = 1.777778f;

	LensSettings.MinFocalLength = 4.0f;
	LensSettings.MaxFocalLength = 1000.0f;
	LensSettings.MinFStop = 1.2f;
	LensSettings.MaxFStop = 22.0f;
	LensSettings.SqueezeFactor = 1.0f;
	LensSettings.DiaphragmBladeCount = 7;
}

void UDialogCameraShot::Play(UDialogBuilderGraph* InDialog, AActor* InSpeaker, TArray<AActor*> InListeners)
{
	if (!GetWorld() ||
        !InDialog)
		return;

	ACineCameraActor* Cinecam = InDialog->Cinecam.Get();

	//Set camera setting
	if (UCineCameraComponent* CinecamComp = InDialog->Cinecam->GetCineCameraComponent())
	{
		CinecamComp->CropSettings = CropSettings;
		CinecamComp->SetCurrentFocalLength(FocalLengthOverride);
		CinecamComp->SetCurrentAperture(ApertureOverride);
		CinecamComp->SetFilmback(Filmback);
		CinecamComp->SetLensSettings(LensSettings);
		if (bUsePostProcess)
		{
			CinecamComp->PostProcessSettings = PostProcessSettings;
		}
	}

	// Set the current view target to the cinecam
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
    {
		PC->SetViewTarget(const_cast<ACineCameraActor*>(Cinecam));
	}

    //TODO : Determine lookatactor, meanwhile only speaker is the lookat actor
    LookAtActor = InSpeaker;

    // Optionally, focus/track the speaker if LookAtActor is valid
    if (Cinecam && LookAtActor.IsValid())
    {
        // Replace this block in UDialogBuilderGraph::PlayDialogShot
        const FVector Offset = LookAtActor->GetActorTransform().InverseTransformPosition(InDialog->GetSpeakerBoneLocation(LookAtActor.Get()));
        const FVector ActorLocation = LookAtActor->GetActorLocation() + Offset;
        const FRotator ActorRotation = LookAtActor->GetActorRotation();
        const FVector ForwardVector = ActorRotation.Vector();

		FVector LocationOverride;
		FRotator RotationOverride;
		float RandomAngleX;
		float RandomAngleY;

       

		switch (CameraAngleRule)
		{
			case(EDialogCameraAngleRule::E_RandomCone):
				// Parameters for cone
				RandomAngleX = FMath::FRandRange(-ConeAngle.X, ConeAngle.X);
				RandomAngleY = FMath::FRandRange(-ConeAngle.Y, ConeAngle.Y);

				// Calculate random direction within cone
				FRotator OffsetRot = FRotator(RandomAngleY, ActorRotation.Yaw + RandomAngleX, 0.f);
				FVector ConeDirection = OffsetRot.Vector();

				LocationOverride = ActorLocation + ConeDirection * CameraDistance;
				RotationOverride = ActorRotation;
				break;
            case(EDialogCameraAngleRule::E_Custom):
				// Third-person camera style: position camera behind and slightly above the actor, matching CameraAngle
				

				// Calculate direction based on angle
				FRotator CameraRot = FRotator(CameraAngle.Y, ActorRotation.Yaw + CameraAngle.X, 0.f);
				FVector Direction = CameraRot.Vector();

				LocationOverride = ActorLocation + Direction * CameraDistance;
				RotationOverride = (ActorLocation - LocationOverride).Rotation();
            break;
		}

        //make sure to generate camera only if shot get overidden or the last speaker is different
        if (InDialog->CachedLastShot != this)
        {
            Cinecam->SetActorLocation(LocationOverride);
            Cinecam->LookatTrackingSettings.bEnableLookAtTracking = true;
            Cinecam->LookatTrackingSettings.ActorToTrack = LookAtActor.Get();
            Cinecam->LookatTrackingSettings.RelativeOffset = Offset + FVector(10.f, 0.f, 0.f);
        }

    }
}
