// Copyright 2025 Ivan Chandra. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "../../Library/OrionCharacterEnumLibrary.h"

#include "OrionPlayerCameraBehavior.generated.h"

class AOrionBaseCharacter;
class AOrionPlayerController;

/**
 * Main class for player camera movement behavior
 */
UCLASS(Blueprintable, BlueprintType)
class ORIONCOMBATFRAMEWORK_API UOrionPlayerCameraBehavior : public UAnimInstance
{
	GENERATED_BODY()

public:
	void SetRotationMode(EOrionRotationMode RotationMode);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	EOrionMovementState MovementState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	EOrionMovementAction MovementAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	bool bLookingDirection = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	bool bVelocityDirection = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	bool bAiming = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	EOrionGait Gait;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	EOrionStance Stance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	EOrionViewMode ViewMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	bool bRightShoulder = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Read Only Data|Character Information")
	bool bDebugView = false;
};
