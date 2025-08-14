// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "../Input/OrionInputConfig.h"
#include "OrionPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UOrionInputConfig;
class UEnhancedInputComponent;
class UOrionAbilitySystemComponent;
class USplineComponent;
class UDamageTextComponent;
class AOrionCharacterBase;

class IEnemyInterface;

struct FInputActionValue;

UCLASS(Blueprintable, BlueprintType)
class ORIONCOMBATFRAMEWORK_API AOrionPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AOrionPlayerController();

	virtual void BeginPlay() override;

	virtual void PlayerTick(float DeltaTime) override;

	virtual void OnPossess(APawn* NewPawn) override;

	virtual void OnRep_Pawn() override;

	virtual void SetupInputComponent() override;

	virtual void BindActions(UInputMappingContext* Context);

	UFUNCTION(Client, Reliable)
	void ShowDamageNumber(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit);
protected:
	void SetupInputs();

	void SetupCamera();

	UFUNCTION()
	void ForwardMovementAction(const FInputActionValue& Value);

	UFUNCTION()
	void RightMovementAction(const FInputActionValue& Value);

	UFUNCTION()
	void CameraUpAction(const FInputActionValue& Value);

	UFUNCTION()
	void CameraRightAction(const FInputActionValue& Value);

	UFUNCTION()
	void JumpAction(const FInputActionValue& Value);

	UFUNCTION()
	void SprintAction(const FInputActionValue& Value);

	UFUNCTION()
	void AimAction(const FInputActionValue& Value);

	UFUNCTION()
	void CameraTapAction(const FInputActionValue& Value);

	UFUNCTION()
	void CameraHeldAction(const FInputActionValue& Value);

	UFUNCTION()
	void StanceAction(const FInputActionValue& Value);

	UFUNCTION()
	void WalkAction(const FInputActionValue& Value);

	UFUNCTION()
	void RagdollAction(const FInputActionValue& Value);

	UFUNCTION()
	void VelocityDirectionAction(const FInputActionValue& Value);

	UFUNCTION()
	void LookingDirectionAction(const FInputActionValue& Value);

	// Debug actions
	UFUNCTION()
	void DebugToggleHudAction(const FInputActionValue& Value);

	UFUNCTION()
	void DebugToggleDebugViewAction(const FInputActionValue& Value);

	UFUNCTION()
	void DebugToggleTracesAction(const FInputActionValue& Value);

	UFUNCTION()
	void DebugToggleShapesAction(const FInputActionValue& Value);

	UFUNCTION()
	void DebugToggleLayerColorsAction(const FInputActionValue& Value);

	UFUNCTION()
	void DebugToggleCharacterInfoAction(const FInputActionValue& Value);

	UFUNCTION()
	void DebugToggleSlomoAction(const FInputActionValue& Value);

	UFUNCTION()
	void DebugFocusedCharacterCycleAction(const FInputActionValue& Value);

	UFUNCTION()
	void DebugToggleMeshAction(const FInputActionValue& Value);

	UFUNCTION()
	void DebugOpenOverlayMenuAction(const FInputActionValue& Value);

	UFUNCTION()
	void DebugOverlayMenuCycleAction(const FInputActionValue& Value);

private:
	void CursorTrace();

	TScriptInterface<IEnemyInterface> LastActor;
	TScriptInterface<IEnemyInterface> ThisActor; 
	FHitResult CursorHit;

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UOrionInputConfig> InputConfig;

	UPROPERTY()
	TObjectPtr<UOrionAbilitySystemComponent> OrionAbilitySystemComponent;

	UOrionAbilitySystemComponent* GetASC();

	FVector CachedDestination = FVector::ZeroVector;
	float FollowTime = 0.f;
	float ShortPressThreshold = 0.5f;
	bool bTargeting = false;




	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageTextComponent> DamageTextComponentClass;
public:
    //Input
    template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
    void BindAbilityActions(UEnhancedInputComponent* EnhancedInputComponent, const UOrionInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc);

public:
	/** Main character reference */
	UPROPERTY(BlueprintReadOnly, Category = "Orion")
	TObjectPtr<AOrionCharacterBase> PossessedCharacter = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputMappingContext> DebugInputMappingContext = nullptr;
};



