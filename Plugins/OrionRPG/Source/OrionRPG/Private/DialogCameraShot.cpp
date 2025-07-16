// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "DialogCameraShot.h"

UDialogCameraShot::UDialogCameraShot()
{
	bUsePostProcess = false;
	CameraDistance = 100.f;
	FocalLengthOverride = 35.0f;
	ApertureOverride = 2.8f;

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
