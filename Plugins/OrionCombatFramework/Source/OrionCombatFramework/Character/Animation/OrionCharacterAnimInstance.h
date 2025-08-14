// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "../../Library/OrionAnimationStructLibrary.h"
#include "../../Library/OrionStructEnumLibrary.h"

#include "OrionCharacterAnimInstance.generated.h"

// forward declarations
class UOrionDebugComponent;
class AOrionCharacterBase;
class UCurveFloat;
class UAnimSequence;
class UCurveVector;

/**
 * Main anim instance class for character
 */
UCLASS(Blueprintable, BlueprintType)
class ORIONCOMBATFRAMEWORK_API UOrionCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeBeginPlay() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Orion|Animation")
	void PlayTransition(const FOrionDynamicMontageParams& Parameters);

	UFUNCTION(BlueprintCallable, Category = "Orion|Animation")
	void PlayTransitionChecked(const FOrionDynamicMontageParams& Parameters);

	UFUNCTION(BlueprintCallable, Category = "Orion|Animation")
	void PlayDynamicTransition(float ReTriggerDelay, FOrionDynamicMontageParams Parameters);

	UFUNCTION(BlueprintCallable, Category = "Orion|Event")
	void OnJumped();

	UFUNCTION(BlueprintCallable, Category = "Orion|Event")
	void OnPivot();

protected:

	UFUNCTION(BlueprintCallable, Category = "Orion|Grounded")
	void SetTrackedHipsDirection(EOrionHipsDirection HipsDirection)
	{
		Grounded.TrackedHipsDirection = HipsDirection;
	}

	UFUNCTION(BlueprintCallable, Category = "Orion|Grounded")
	void SetGroundedEntryState(EOrionGroundedEntryState NewState)
	{
		GroundedEntryState = NewState;
	}

	/** Enable Movement Animations if IsMoving and HasMovementInput, or if the Speed is greater than 150. */
	UFUNCTION(BlueprintCallable, Category = "Orion|Grounded")
	bool ShouldMoveCheck() const;

	/** Only perform a Rotate In Place Check if the character is Aiming or in First Person. */
	UFUNCTION(BlueprintCallable, Category = "Orion|Grounded")
	bool CanRotateInPlace() const;

	/**
	 * Only perform a Turn In Place check if the character is looking toward the camera in Third Person,
	 * and if the "Enable Transition" curve is fully weighted. The Enable_Transition curve is modified within certain
	 * states of the AnimBP so that the character can only turn while in those states..
	 */
	UFUNCTION(BlueprintCallable, Category = "Orion|Grounded")
	bool CanTurnInPlace() const;

	/**
	 * Only perform a Dynamic Transition check if the "Enable Transition" curve is fully weighted.
	 * The Enable_Transition curve is modified within certain states of the AnimBP so
	 * that the character can only transition while in those states.
	 */
	UFUNCTION(BlueprintCallable, Category = "Orion|Grounded")
	bool CanDynamicTransition() const;

private:
	void PlayDynamicTransitionDelay();

	void OnJumpedDelay();

	void OnPivotDelay();

	/** Update Values */

	void UpdateAimingValues(float DeltaSeconds);

	void UpdateLayerValues();

	void UpdateFootIK(float DeltaSeconds);

	void UpdateMovementValues(float DeltaSeconds);

	void UpdateRotationValues();

	void UpdateInAirValues(float DeltaSeconds);

	void UpdateRagdollValues();

	/** Foot IK */

	void SetFootLocking(float DeltaSeconds, FName EnableFootIKCurve, FName FootLockCurve, FName IKFootBone,
                          float& CurFootLockAlpha, bool& UseFootLockCurve,
                          FVector& CurFootLockLoc, FRotator& CurFootLockRot);

	void SetFootLockOffsets(float DeltaSeconds, FVector& LocalLoc, FRotator& LocalRot);

	void SetPelvisIKOffset(float DeltaSeconds, FVector FootOffsetLTarget, FVector FootOffsetRTarget);

	void ResetIKOffsets(float DeltaSeconds);

	void SetFootOffsets(float DeltaSeconds, FName EnableFootIKCurve, FName IKFootBone, FName RootBone,
                          FVector& CurLocationTarget, FVector& CurLocationOffset, FRotator& CurRotationOffset);

	/** Grounded */

	void RotateInPlaceCheck();

	void TurnInPlaceCheck(float DeltaSeconds);

	void DynamicTransitionCheck();

	FOrionVelocityBlend CalculateVelocityBlend() const;

	void TurnInPlace(FRotator TargetRotation, float PlayRateScale, float StartTime, bool OverrideCurrent);

	/** Movement */

	FVector CalculateRelativeAccelerationAmount() const;

	float CalculateStrideBlend() const;

	float CalculateWalkRunBlend() const;

	float CalculateStandingPlayRate() const;

	float CalculateDiagonalScaleAmount() const;

	float CalculateCrouchingPlayRate() const;

	float CalculateLandPrediction() const;

	FOrionLeanAmount CalculateAirLeanAmount() const;

	EOrionMovementDirection CalculateMovementDirection() const;

	/** Util */

	float GetAnimCurveClamped(const FName& Name, float Bias, float ClampMin, float ClampMax) const;

public:
	/** References */
	UPROPERTY(BlueprintReadOnly, Category = "Read Only Data|Character Information")
	TObjectPtr<AOrionCharacterBase> Character = nullptr;

	/** Character Information */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information", Meta = (
		ShowOnlyInnerProperties))
	FOrionAnimCharacterInformation CharacterInformation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FOrionMovementState MovementState = EOrionMovementState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FOrionMovementAction MovementAction = EOrionMovementAction::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FOrionRotationMode RotationMode = EOrionRotationMode::VelocityDirection;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FOrionGait Gait = EOrionGait::Walking;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FOrionStance Stance = EOrionStance::Standing;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FOrionOverlayState OverlayState = EOrionOverlayState::Default;

	/** Anim Graph - Grounded */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded", Meta = (
		ShowOnlyInnerProperties))
	FOrionAnimGraphGrounded Grounded;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FOrionVelocityBlend VelocityBlend;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FOrionLeanAmount LeanAmount;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FVector RelativeAccelerationAmount = FVector::ZeroVector;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FOrionGroundedEntryState GroundedEntryState = EOrionGroundedEntryState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FOrionMovementDirection MovementDirection = EOrionMovementDirection::Forward;

	/** Anim Graph - In Air */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - In Air", Meta = (
		ShowOnlyInnerProperties))
	FOrionAnimGraphInAir InAir;

	/** Anim Graph - Aiming Values */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Aiming Values", Meta = (
		ShowOnlyInnerProperties))
	FOrionAnimGraphAimingValues AimingValues;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Aiming Values")
	FVector2D SmoothedAimingAngle = FVector2D::ZeroVector;

	/** Anim Graph - Ragdoll */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Ragdoll")
	float FlailRate = 0.0f;

	/** Anim Graph - Layer Blending */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Layer Blending", Meta = (
		ShowOnlyInnerProperties))
	FOrionAnimGraphLayerBlending LayerBlendingValues;

	/** Anim Graph - Foot IK */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Foot IK", Meta = (
		ShowOnlyInnerProperties))
	FOrionAnimGraphFootIK FootIKValues;

	/** Turn In Place */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Turn In Place", Meta = (
		ShowOnlyInnerProperties))
	FOrionAnimTurnInPlace TurnInPlaceValues;

	/** Rotate In Place */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Rotate In Place", Meta = (
		    ShowOnlyInnerProperties))
	FOrionAnimRotateInPlace RotateInPlace;

	/** Configuration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Main Configuration", Meta = (
		ShowOnlyInnerProperties))
	FOrionAnimConfiguration Config;

	/** Blend Curves */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> DiagonalScaleAmountCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> StrideBlend_N_Walk = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> StrideBlend_N_Run = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> StrideBlend_C_Walk = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> LandPredictionCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> LeanInAirCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveVector> YawOffset_FB = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveVector> YawOffset_LR = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Dynamic Transition")
	TObjectPtr<UAnimSequenceBase> TransitionAnim_R = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Dynamic Transition")
	TObjectPtr<UAnimSequenceBase> TransitionAnim_L = nullptr;

	/** IK Bone Names */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Anim Graph - Foot IK")
	FName IkFootL_BoneName = FName(TEXT("ik_foot_l"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Anim Graph - Foot IK")
	FName IkFootR_BoneName = FName(TEXT("ik_foot_r"));

private:
	FTimerHandle OnPivotTimer;

	FTimerHandle PlayDynamicTransitionTimer;

	FTimerHandle OnJumpedTimer;

	bool bCanPlayDynamicTransition = true;

	UPROPERTY()
	TObjectPtr<UOrionDebugComponent> OrionDebugComponent = nullptr;
};
