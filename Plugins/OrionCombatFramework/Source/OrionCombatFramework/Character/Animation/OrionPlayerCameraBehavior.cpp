// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "OrionPlayerCameraBehavior.h"
#include "../../Character/OrionCharacterBase.h"

void UOrionPlayerCameraBehavior::SetRotationMode(EOrionRotationMode RotationMode)
{
	bVelocityDirection = RotationMode == EOrionRotationMode::VelocityDirection;
	bLookingDirection = RotationMode == EOrionRotationMode::LookingDirection;
	bAiming = RotationMode == EOrionRotationMode::Aiming;
}
