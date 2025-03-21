// Fill out your copyright notice in the Description page of Project Settings.

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

class IEnemyInterface;

struct FInputActionValue;

UCLASS()
class ORIONCOMBATFRAMEWORK_API AOrionPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AOrionPlayerController();
	virtual void PlayerTick(float DeltaTime) override;

	UFUNCTION(Client, Reliable)
	void ShowDamageNumber(float DamageAmount, ACharacter* TargetCharacter);
protected:
	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;
private:
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> OrionContext;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ShiftAction;

	void ShiftPressed() { bShiftKeyDown = true; };
	void ShiftReleased() { bShiftKeyDown = false; };
	bool bShiftKeyDown = false;

	void Move(const FInputActionValue& InputActionValue);

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
	bool bAutoRunning = false;
	bool bTargeting = false;

	UPROPERTY(EditDefaultsOnly)
	float AutoRunAcceptanceRadius = 50.f;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> Spline;

	void AutoRun();


	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageTextComponent> DamageTextComponentClass;
public:
    //Input
    template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
    void BindAbilityActions(UEnhancedInputComponent* EnhancedInputComponent, const UOrionInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc);
};



