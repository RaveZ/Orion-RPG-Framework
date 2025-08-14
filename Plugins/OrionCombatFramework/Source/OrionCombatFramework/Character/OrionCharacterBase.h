// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../Library/OrionCharacterEnumLibrary.h"
#include "../Library/OrionCharacterStructLibrary.h"
#include "../Interaction/CombatInterface.h"
#include "../Progression/Reward.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "AbilitySystemInterface.h"
#include "ScalableFloat.h"
#include "Engine/DataTable.h"
#include "OrionCharacterBase.generated.h"

// forward declarations
class UAnimMontage;
class UGameplayEffect;
class UAttributeSet;
class UAbilitySystemComponent;
class UGameplayAbility;
class UOrionDebugComponent;
class UAnimMontage;
class UOrionPlayerCameraBehavior;
enum class EVisibilityBasedAnimTickOption : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FJumpPressedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJumpedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRagdollStateChangedSignature, bool, bRagdollState);

UCLASS(Abstract, BlueprintType)
class ORIONCOMBATFRAMEWORK_API AOrionCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	AOrionCharacterBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Orion|Movement")
	FORCEINLINE class UOrionCharacterMovementComponent* GetMyMovementComponent() const
	{
		return MyCharacterMovementComponent;
	}

	virtual void Tick(float DeltaTime) override;

	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region CORE GAMEPLAY
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	virtual UAnimMontage* GetHitReactMontage_Implementation() override;

	virtual void Die() override;

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath();


protected:

	UPROPERTY(EditAnywhere, Category = "Orion | Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

	UPROPERTY(EditAnywhere, Category = "Orion | Combat")
	FName WeaponTipSocketName;

	virtual FVector GetCombatSocketLocation() override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	virtual void InitAbilityActorInfo();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Orion | Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Orion | Attributes")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Orion | Attributes")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;

	/** Reward granted to the source character upon defeating this character */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Orion | Progression")
	TArray<FRewardInfo> DefeatReward;

	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;

	void InitializeDefaultAttributes() const;

	void AddCharacterAbilities();

	/* Dissolve Effects */

	void Dissolve();

	UFUNCTION(BlueprintImplementableEvent)
	void StartDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);

	UFUNCTION(BlueprintImplementableEvent)
	void StartWeaponDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstance;
private:

	UPROPERTY(EditAnywhere, Category = "Orion | Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	UPROPERTY(EditAnywhere, Category = "Orion | Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupPassiveAbilities;

	UPROPERTY(EditAnywhere, Category = "Orion |Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;

public:
	FORCEINLINE TArray<FRewardInfo> GetDefeatReward() const { return DefeatReward; }
#pragma endregion


#pragma region CORE CHARACTER
public:
	/** Ragdoll System */

	/** Implement on BP to get required get up animation according to character's state */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Orion|Ragdoll System")
	UAnimMontage* GetGetUpAnimation(bool bRagdollFaceUpState);

	UFUNCTION(BlueprintCallable, Category = "Orion|Ragdoll System")
	virtual void RagdollStart();

	UFUNCTION(BlueprintCallable, Category = "Orion|Ragdoll System")
	virtual void RagdollEnd();

	UFUNCTION(BlueprintCallable, Server, Unreliable, Category = "Orion|Ragdoll System")
	void Server_SetMeshLocationDuringRagdoll(FVector MeshLocation);

	/** Character States */

	UFUNCTION(BlueprintCallable, Category = "Orion|Character States")
	void SetMovementState(EOrionMovementState NewState, bool bForce = false);

	UFUNCTION(BlueprintGetter, Category = "Orion|Character States")
	EOrionMovementState GetMovementState() const { return MovementState; }

	UFUNCTION(BlueprintGetter, Category = "Orion|Character States")
	EOrionMovementState GetPrevMovementState() const { return PrevMovementState; }

	UFUNCTION(BlueprintCallable, Category = "Orion|Character States")
	void SetMovementAction(EOrionMovementAction NewAction, bool bForce = false);

	UFUNCTION(BlueprintGetter, Category = "Orion|Character States")
	EOrionMovementAction GetMovementAction() const { return MovementAction; }

	UFUNCTION(BlueprintCallable, Category = "Orion|Character States")
	void SetStance(EOrionStance NewStance, bool bForce = false);

	UFUNCTION(BlueprintGetter, Category = "Orion|Character States")
	EOrionStance GetStance() const { return Stance; }

	UFUNCTION(BlueprintCallable, Category = "Orion|Character States")
	void SetOverlayOverrideState(int32 NewState);

	UFUNCTION(BlueprintGetter, Category = "Orion|Character States")
	int32 GetOverlayOverrideState() const { return OverlayOverrideState; }

	UFUNCTION(BlueprintCallable, Category = "Orion|Character States")
	void SetGait(EOrionGait NewGait, bool bForce = false);

	UFUNCTION(BlueprintGetter, Category = "Orion|Character States")
	EOrionGait GetGait() const { return Gait; }

	UFUNCTION(BlueprintGetter, Category = "Orion|CharacterStates")
	EOrionGait GetDesiredGait() const { return DesiredGait; }

	UFUNCTION(BlueprintCallable, Category = "Orion|Character States")
	void SetRotationMode(EOrionRotationMode NewRotationMode, bool bForce = false);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Orion|Character States")
	void Server_SetRotationMode(EOrionRotationMode NewRotationMode, bool bForce);

	UFUNCTION(BlueprintGetter, Category = "Orion|Character States")
	EOrionRotationMode GetRotationMode() const { return RotationMode; }

	UFUNCTION(BlueprintCallable, Category = "Orion|Character States")
	void SetViewMode(EOrionViewMode NewViewMode, bool bForce = false);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Orion|Character States")
	void Server_SetViewMode(EOrionViewMode NewViewMode, bool bForce);

	UFUNCTION(BlueprintGetter, Category = "Orion|Character States")
	EOrionViewMode GetViewMode() const { return ViewMode; }

	UFUNCTION(BlueprintCallable, Category = "Orion|Character States")
	void SetOverlayState(EOrionOverlayState NewState, bool bForce = false);

	UFUNCTION(BlueprintCallable, Category = "Orion|Character States")
	void SetGroundedEntryState(EOrionGroundedEntryState NewState);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Orion|Character States")
	void Server_SetOverlayState(EOrionOverlayState NewState, bool bForce);

	UFUNCTION(BlueprintGetter, Category = "Orion|Character States")
	EOrionOverlayState GetOverlayState() const { return OverlayState; }

	UFUNCTION(BlueprintGetter, Category = "Orion|Character States")
	EOrionGroundedEntryState GetGroundedEntryState() const { return GroundedEntryState; }

	/** Landed, Jumped, Rolling, Mantling and Ragdoll*/
	/** On Landed*/
	UFUNCTION(BlueprintCallable, Category = "Orion|Character States")
	void EventOnLanded();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Orion|Character States")
	void Multicast_OnLanded();

	/** On Jumped*/
	UFUNCTION(BlueprintCallable, Category = "Orion|Character States")
	void EventOnJumped();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Orion|Character States")
	void Multicast_OnJumped();

	/** Rolling Montage Play Replication*/
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Orion|Character States")
	void Server_PlayMontage(UAnimMontage* Montage, float PlayRate);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Orion|Character States")
	void Multicast_PlayMontage(UAnimMontage* Montage, float PlayRate);

	/** Ragdolling*/
	UFUNCTION(BlueprintCallable, Category = "Orion|Character States")
	void ReplicatedRagdollStart();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Orion|Character States")
	void Server_RagdollStart();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Orion|Character States")
	void Multicast_RagdollStart();

	UFUNCTION(BlueprintCallable, Category = "Orion|Character States")
	void ReplicatedRagdollEnd();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Orion|Character States")
	void Server_RagdollEnd(FVector CharacterLocation);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Orion|Character States")
	void Multicast_RagdollEnd(FVector CharacterLocation);

	/** Input */

	UPROPERTY(BlueprintAssignable, Category = "Orion|Input")
	FJumpPressedSignature JumpPressedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Orion|Input")
	FOnJumpedSignature OnJumpedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Orion|Input")
	FRagdollStateChangedSignature RagdollStateChangedDelegate;

	UFUNCTION(BlueprintGetter, Category = "Orion|Input")
	EOrionStance GetDesiredStance() const { return DesiredStance; }

	UFUNCTION(BlueprintSetter, Category = "Orion|Input")
	void SetDesiredStance(EOrionStance NewStance);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Orion|Input")
	void Server_SetDesiredStance(EOrionStance NewStance);

	UFUNCTION(BlueprintCallable, Category = "Orion|Character States")
	void SetDesiredGait(EOrionGait NewGait);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Orion|Character States")
	void Server_SetDesiredGait(EOrionGait NewGait);

	UFUNCTION(BlueprintGetter, Category = "Orion|Input")
	EOrionRotationMode GetDesiredRotationMode() const { return DesiredRotationMode; }

	UFUNCTION(BlueprintSetter, Category = "Orion|Input")
	void SetDesiredRotationMode(EOrionRotationMode NewRotMode);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Orion|Character States")
	void Server_SetDesiredRotationMode(EOrionRotationMode NewRotMode);

	/** Rotation System */

	UFUNCTION(BlueprintCallable, Category = "Orion|Rotation System")
	void SetActorLocationAndTargetRotation(FVector NewLocation, FRotator NewRotation);

	/** Movement System */

	UFUNCTION(BlueprintGetter, Category = "Orion|Movement System")
	bool HasMovementInput() const { return bHasMovementInput; }

	UFUNCTION(BlueprintCallable, Category = "Orion|Movement System")
	FOrionMovementSettings GetTargetMovementSettings() const;

	UFUNCTION(BlueprintCallable, Category = "Orion|Movement System")
	EOrionGait GetAllowedGait() const;

	UFUNCTION(BlueprintCallable, Category = "Orion|Movement System")
	EOrionGait GetActualGait(EOrionGait AllowedGait) const;

	UFUNCTION(BlueprintCallable, Category = "Orion|Movement System")
	bool CanSprint() const;

	/** BP implementable function that called when Breakfall starts */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Orion|Movement System")
	void OnBreakfall();
	virtual void OnBreakfall_Implementation();

	/** BP implementable function that called when A Montage starts, e.g. during rolling */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Orion|Movement System")
	void Replicated_PlayMontage(UAnimMontage* Montage, float PlayRate);
	virtual void Replicated_PlayMontage_Implementation(UAnimMontage* Montage, float PlayRate);

	/** Implement on BP to get required roll animation according to character's state */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Orion|Movement System")
	UAnimMontage* GetRollAnimation();

	/** Utility */

	UFUNCTION(BlueprintCallable, Category = "Orion|Utility")
	float GetAnimCurveValue(FName CurveName) const;

	UFUNCTION(BlueprintCallable, Category = "Orion|Utility")
	void SetVisibleMesh(USkeletalMesh* NewSkeletalMesh);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Orion|Utility")
	void Server_SetVisibleMesh(USkeletalMesh* NewSkeletalMesh);

	/** Camera System */

	UFUNCTION(BlueprintGetter, Category = "Orion|Camera System")
	bool IsRightShoulder() const { return bRightShoulder; }

	UFUNCTION(BlueprintCallable, Category = "Orion|Camera System")
	void SetRightShoulder(bool bNewRightShoulder);

	UFUNCTION(BlueprintCallable, Category = "Orion|Camera System")
	virtual ECollisionChannel GetThirdPersonTraceParams(FVector& TraceOrigin, float& TraceRadius);

	UFUNCTION(BlueprintCallable, Category = "Orion|Camera System")
	virtual FTransform GetThirdPersonPivotTarget();

	UFUNCTION(BlueprintCallable, Category = "Orion|Camera System")
	virtual FVector GetFirstPersonCameraTarget();

	UFUNCTION(BlueprintCallable, Category = "Orion|Camera System")
	void GetCameraParameters(float& TPFOVOut, float& FPFOVOut, bool& bRightShoulderOut) const;

	UFUNCTION(BlueprintCallable, Category = "Orion|Camera System")
	void SetCameraBehavior(UOrionPlayerCameraBehavior* CamBeh) { CameraBehavior = CamBeh; }

	/** Essential Information Getters/Setters */

	UFUNCTION(BlueprintGetter, Category = "Orion|Essential Information")
	FVector GetAcceleration() const { return Acceleration; }

	UFUNCTION(BlueprintGetter, Category = "Orion|Essential Information")
	bool IsMoving() const { return bIsMoving; }

	UFUNCTION(BlueprintCallable, Category = "Orion|Essential Information")
	FVector GetMovementInput() const;

	UFUNCTION(BlueprintGetter, Category = "Orion|Essential Information")
	float GetMovementInputAmount() const { return MovementInputAmount; }

	UFUNCTION(BlueprintGetter, Category = "Orion|Essential Information")
	float GetSpeed() const { return Speed; }

	UFUNCTION(BlueprintCallable, Category = "Orion|Essential Information")
	FRotator GetAimingRotation() const { return AimingRotation; }

	UFUNCTION(BlueprintGetter, Category = "Orion|Essential Information")
	float GetAimYawRate() const { return AimYawRate; }

	/** Input */

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Orion|Input")
	void ForwardMovementAction(float Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Orion|Input")
	void RightMovementAction(float Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Orion|Input")
	void CameraUpAction(float Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Orion|Input")
	void CameraRightAction(float Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Orion|Input")
	void JumpAction(bool bValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Orion|Input")
	void SprintAction(bool bValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Orion|Input")
	void AimAction(bool bValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Orion|Input")
	void CameraTapAction();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Orion|Input")
	void CameraHeldAction();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Orion|Input")
	void StanceAction();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Orion|Input")
	void WalkAction();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Orion|Input")
	void RagdollAction();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Orion|Input")
	void VelocityDirectionAction();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Orion|Input")
	void LookingDirectionAction();

protected:
	/** Ragdoll System */

	void RagdollUpdate(float DeltaTime);

	void SetActorLocationDuringRagdoll(float DeltaTime);

	/** State Changes */

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	virtual void OnMovementStateChanged(EOrionMovementState PreviousState);

	virtual void OnMovementActionChanged(EOrionMovementAction PreviousAction);

	virtual void OnStanceChanged(EOrionStance PreviousStance);

	virtual void OnRotationModeChanged(EOrionRotationMode PreviousRotationMode);

	virtual void OnGaitChanged(EOrionGait PreviousGait);

	virtual void OnViewModeChanged(EOrionViewMode PreviousViewMode);

	virtual void OnOverlayStateChanged(EOrionOverlayState PreviousState);

	virtual void OnVisibleMeshChanged(const USkeletalMesh* PreviousSkeletalMesh);

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual void OnJumped_Implementation() override;

	virtual void Landed(const FHitResult& Hit) override;

	void OnLandFrictionReset();

	void SetEssentialValues(float DeltaTime);

	void UpdateCharacterMovement();

	void UpdateGroundedRotation(float DeltaTime);

	void UpdateInAirRotation(float DeltaTime);

	/** Utils */

	void SmoothCharacterRotation(FRotator Target, float TargetInterpSpeed, float ActorInterpSpeed, float DeltaTime);

	float CalculateGroundedRotationRate() const;

	void LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed, float DeltaTime);

	void SetMovementModel();

	void ForceUpdateCharacterState();

	/** Replication */
	UFUNCTION(Category = "Orion|Replication")
	void OnRep_RotationMode(EOrionRotationMode PrevRotMode);

	UFUNCTION(Category = "Orion|Replication")
	void OnRep_ViewMode(EOrionViewMode PrevViewMode);

	UFUNCTION(Category = "Orion|Replication")
	void OnRep_OverlayState(EOrionOverlayState PrevOverlayState);

	UFUNCTION(Category = "Orion|Replication")
	void OnRep_VisibleMesh(const USkeletalMesh* PreviousSkeletalMesh);

protected:
	/* Custom movement component*/
	UPROPERTY()
	TObjectPtr<UOrionCharacterMovementComponent> MyCharacterMovementComponent;

	/** Input */

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Orion|Input")
	EOrionRotationMode DesiredRotationMode = EOrionRotationMode::LookingDirection;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Orion|Input")
	EOrionGait DesiredGait = EOrionGait::Running;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Orion|Input")
	EOrionStance DesiredStance = EOrionStance::Standing;

	UPROPERTY(EditDefaultsOnly, Category = "Orion|Input", BlueprintReadOnly)
	float LookUpDownRate = 1.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Orion|Input", BlueprintReadOnly)
	float LookLeftRightRate = 1.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Orion|Input", BlueprintReadOnly)
	float RollDoubleTapTimeout = 0.3f;

	UPROPERTY(Category = "Orion|Input", BlueprintReadOnly)
	bool bBreakFall = false;

	UPROPERTY(Category = "Orion|Input", BlueprintReadOnly)
	bool bSprintHeld = false;

	/** Camera System */

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Orion|Camera System")
	float ThirdPersonFOV = 90.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Orion|Camera System")
	float FirstPersonFOV = 90.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Orion|Camera System")
	bool bRightShoulder = false;

	/** Movement System */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Orion|Movement System")
	FDataTableRowHandle MovementModel;

	/** Essential Information */

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Essential Information")
	FVector Acceleration = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Essential Information")
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Essential Information")
	bool bHasMovementInput = false;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Essential Information")
	FRotator LastVelocityRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Essential Information")
	FRotator LastMovementInputRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Essential Information")
	float Speed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Essential Information")
	float MovementInputAmount = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Essential Information")
	float AimYawRate = 0.0f;

	/** Replicated Essential Information*/

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Essential Information")
	float EasedMaxAcceleration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Orion|Essential Information")
	FVector ReplicatedCurrentAcceleration = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Orion|Essential Information")
	FRotator ReplicatedControlRotation = FRotator::ZeroRotator;

	/** Replicated Skeletal Mesh Information*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Orion|Skeletal Mesh", ReplicatedUsing = OnRep_VisibleMesh)
	TObjectPtr<USkeletalMesh> VisibleMesh = nullptr;

	/** State Values */

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Orion|State Values", ReplicatedUsing = OnRep_OverlayState)
	EOrionOverlayState OverlayState = EOrionOverlayState::Default;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|State Values")
	EOrionGroundedEntryState GroundedEntryState;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|State Values")
	EOrionMovementState MovementState = EOrionMovementState::None;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|State Values")
	EOrionMovementState PrevMovementState = EOrionMovementState::None;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|State Values")
	EOrionMovementAction MovementAction = EOrionMovementAction::None;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|State Values", ReplicatedUsing = OnRep_RotationMode)
	EOrionRotationMode RotationMode = EOrionRotationMode::LookingDirection;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|State Values")
	EOrionGait Gait = EOrionGait::Walking;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Orion|State Values")
	EOrionStance Stance = EOrionStance::Standing;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Orion|State Values", ReplicatedUsing = OnRep_ViewMode)
	EOrionViewMode ViewMode = EOrionViewMode::ThirdPerson;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|State Values")
	int32 OverlayOverrideState = 0;

	/** Movement System */

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Movement System")
	FOrionMovementStateSettings MovementData;

	/** Rotation System */

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Rotation System")
	FRotator TargetRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Rotation System")
	FRotator InAirRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Rotation System")
	float YawOffset = 0.0f;

	/** Breakfall System */

	/** If player hits to the ground with a specified amount of velocity, switch to breakfall state */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Orion|Breakfall System")
	bool bBreakfallOnLand = true;

	/** If player hits to the ground with an amount of velocity greater than specified value, switch to breakfall state */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Orion|Breakfall System", meta = (EditCondition = "bBreakfallOnLand"))
	float BreakfallOnLandVelocity = 700.0f;

	/** Ragdoll System */

	/** If the skeleton uses a reversed pelvis bone, flip the calculation operator */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Orion|Ragdoll System")
	bool bReversedPelvis = false;

	/** If player hits to the ground with a specified amount of velocity, switch to ragdoll state */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Orion|Ragdoll System")
	bool bRagdollOnLand = false;

	/** If player hits to the ground with an amount of velocity greater than specified value, switch to ragdoll state */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Orion|Ragdoll System", meta = (EditCondition = "bRagdollOnLand"))
	float RagdollOnLandVelocity = 1000.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Ragdoll System")
	bool bRagdollOnGround = false;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Ragdoll System")
	bool bRagdollFaceUp = false;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Ragdoll System")
	FVector LastRagdollVelocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Orion|Ragdoll System")
	FVector TargetRagdollLocation = FVector::ZeroVector;

	/* Server ragdoll pull force storage*/
	float ServerRagdollPull = 0.0f;

	/* Dedicated server mesh default visibility based anim tick option*/
	EVisibilityBasedAnimTickOption DefVisBasedTickOp;

	bool bPreRagdollURO = false;

	/** Cached Variables */

	FVector PreviousVelocity = FVector::ZeroVector;

	float PreviousAimYaw = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Camera")
	TObjectPtr<UOrionPlayerCameraBehavior> CameraBehavior;

	/** Last time the 'first' crouch/roll button is pressed */
	float LastStanceInputTime = 0.0f;

	/* Timer to manage reset of braking friction factor after on landed event */
	FTimerHandle OnLandedFrictionResetTimer;

	/* Smooth out aiming by interping control rotation*/
	FRotator AimingRotation = FRotator::ZeroRotator;

	/** We won't use curve based movement and a few other features on networked games */
	bool bEnableNetworkOptimizations = false;

private:
	UPROPERTY()
	TObjectPtr<UOrionDebugComponent> OrionDebugComponent = nullptr;
#pragma endregion


};