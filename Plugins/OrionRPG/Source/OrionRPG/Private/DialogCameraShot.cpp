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
	ApertureOverride =  2.8f;
	ConeAngle = FVector2f(30.f, 3.f);
	bOverride_CustomNearClippingPlane = true;
	CustomNearClippingPlane = 15.f;

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

void UDialogCameraShot::Play(UDialogBuilderGraph* InDialog, AActor* InSpeaker)
{
	if (!GetWorld() ||
        !InDialog)
		return;

	K2_PreCameraSetup();

	ACineCameraActor* Cinecam = InDialog->Cinecam.Get();

	//Define Track & Focus rules
	bool bTrackEnabled = CameraTrackingRule == EDialogCameraTrackingRule::E_Disabled ? false : true;
	LookAtActor = CameraTrackingRule == EDialogCameraTrackingRule::E_OtherParticipant ? InDialog->CachedParticipants.FindRef(ParticipantToTrack).Get()
        : InSpeaker;
	
	FocusActor = CameraFocusRule == EDialogCameraFocusRule::E_TrackedActor ? LookAtActor
		: CameraFocusRule == EDialogCameraFocusRule::E_Speaker ? InSpeaker 
		: CameraFocusRule == EDialogCameraFocusRule::E_OtherParticipant ? InDialog->CachedParticipants.FindRef(ParticipantToFocus).Get()
		: nullptr;

	//Set camera setting
	if (UCineCameraComponent* CinecamComp = Cinecam->GetCineCameraComponent())
	{
		CinecamComp->CropSettings = CropSettings;
		CinecamComp->SetCurrentFocalLength(FocalLengthOverride);
		CinecamComp->SetCurrentAperture(ApertureOverride);
		CinecamComp->SetFilmback(Filmback);
		CinecamComp->SetLensSettings(LensSettings);
		CinecamComp->bOverride_CustomNearClippingPlane = bOverride_CustomNearClippingPlane;
		CinecamComp->CustomNearClippingPlane = CustomNearClippingPlane;
		if (bUsePostProcess)
		{
			CinecamComp->PostProcessSettings = PostProcessSettings;
		}


		if (FocusActor.IsValid())
		{
			CinecamComp->FocusSettings.FocusMethod = ECameraFocusMethod::Tracking;
			CinecamComp->FocusSettings.TrackingFocusSettings.RelativeOffset = FocusRelativeOffset;
			CinecamComp->FocusSettings.TrackingFocusSettings.ActorToTrack = FocusActor.Get();
		}
		else
		{
			CinecamComp->FocusSettings.FocusMethod = ECameraFocusMethod::Disable;
		}
	}

	// Set the current view target to the cinecam
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
    {
		PC->SetViewTarget(const_cast<ACineCameraActor*>(Cinecam));
	}


    // Optionally, focus/track the speaker if LookAtActor is valid
    if (Cinecam && LookAtActor.IsValid())
    {
        // Replace this block in UDialogBuilderGraph::PlayDialogShot
        const FVector RelativeOffset = LookAtActor->GetActorTransform().InverseTransformPosition(InDialog->GetSpeakerBoneLocation(LookAtActor.Get()));
        const FVector ActorLocation = LookAtActor->GetActorLocation() + RelativeOffset;
        const FRotator ActorRotation = LookAtActor->GetActorRotation();
        const FVector ForwardVector = ActorRotation.Vector();

		FVector LocationOverride;
		FRotator RotationOverride = FRotator();
		float RandomAngleX;
		float RandomAngleY;

       

		switch (CameraAngleRule)
		{
			case(EDialogCameraAngleRule::E_RandomCone):
				RandomAngleX = FMath::FRandRange(-ConeAngle.X, ConeAngle.X);
				RandomAngleY = FMath::FRandRange(-ConeAngle.Y, ConeAngle.Y);

				// Calculate random direction within cone
				FRotator OffsetRot = FRotator(RandomAngleY, ActorRotation.Yaw + RandomAngleX, 0.f);
				FVector ConeDirection = OffsetRot.Vector();

				LocationOverride = ActorLocation + ConeDirection * CameraDistance;
				RotationOverride = (ActorLocation - LocationOverride).Rotation();
				break;
            case(EDialogCameraAngleRule::E_Custom):
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
			Cinecam->SetActorRotation(RotationOverride);
            Cinecam->LookatTrackingSettings.bEnableLookAtTracking = true;
            Cinecam->LookatTrackingSettings.ActorToTrack = LookAtActor.Get();
            Cinecam->LookatTrackingSettings.RelativeOffset = RelativeOffset + CameraOffset;

			if(!bTrackEnabled)
				Cinecam->LookatTrackingSettings.bEnableLookAtTracking = false;

			
        }

    }
}
