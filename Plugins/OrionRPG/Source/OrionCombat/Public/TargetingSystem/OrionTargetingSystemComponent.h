// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OrionTargetingSystemComponent.generated.h"

class UOrionTargetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrionTargetingSignature, UOrionTargetComponent*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOrionTargetingStateSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrionTargetingEnabledSignature, bool, bEnabled);

UCLASS(meta = (BlueprintSpawnableComponent), Blueprintable, HideDropdown, ClassGroup = "OrionRPG", DisplayName = "Orion Targeting System")
class ORIONCOMBAT_API UOrionTargetingSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOrionTargetingSystemComponent();

	// ---------- Configuration ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	float MaxDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	float SearchRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	float CameraBlendSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TEnumAsByte<ECollisionChannel> TraceChannel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	bool bOnlyTargetsInCameraView;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	float MaxAngleFromCenter;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	float CameraVerticalOffset; // Height offset (cm) to aim at chest/head


	// ---------- Runtime State ----------
	
	//We cache the OwningController, we won't cache pawn as this might change
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<APlayerController> OwningController;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TargetingSystem")
	TObjectPtr<UOrionTargetComponent> CurrentTarget;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TargetingSystem")
	TArray<TObjectPtr<UOrionTargetComponent>> ValidTargetList; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TargetingSystem")
	int32 CurrentCycleIndex = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TargetingSystem")
	bool bTargetingEnabled = true;

	// ---------- Delegates ----------
	UPROPERTY(BlueprintAssignable, Category = "TargetingSystem")
	FOrionTargetingSignature OnLostTarget;

	UPROPERTY(BlueprintAssignable, Category = "TargetingSystem")
	FOrionTargetingSignature OnTargetChanged;

	UPROPERTY(BlueprintAssignable, Category = "TargetingSystem")
	FOrionTargetingSignature OnTargetAcquired;

	UPROPERTY(BlueprintAssignable, Category = "TargetingSystem")
	FOrionTargetingStateSignature OnTargetCleared;

	UPROPERTY(BlueprintAssignable, Category = "TargetingSystem")
	FOrionTargetingEnabledSignature OnTargetingEnabledChanged;

	
public:
	UFUNCTION(BlueprintPure, Category = "TargetingSystem")
	virtual APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintPure, Category = "TargetingSystem")
	virtual APlayerController* GetOwningController() const;

	UFUNCTION()
	virtual void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	
public:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure, Category = "TargetingSystem")
	float GetDistanceToTarget() const;

	UFUNCTION(BlueprintCallable, Category = "TargetingSystem")
	void SetEnableTargeting(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "TargetingSystem")
	void SearchTarget();

	UFUNCTION(BlueprintCallable, Category = "TargetingSystem")
	void CycleToNextTarget();

	UFUNCTION(BlueprintCallable, Category = "TargetingSystem")
	void ClearTarget();

	UFUNCTION(BlueprintPure, Category = "TargetingSystem")
	TArray<UOrionTargetComponent*> GetValidTargets() const;
	
	UFUNCTION(BlueprintPure, Category = "TargetingSystem")
	UOrionTargetComponent* GetNearestValidTarget() const;

protected:
	virtual float ScoreTarget(const UOrionTargetComponent* Target) const;  // const-correct
	virtual bool IsTargetValid(UOrionTargetComponent* Target) const;
	bool HasLineOfSight(const FVector& Start, const FVector& End) const;
	bool IsTargetInCameraView(UOrionTargetComponent* Target, APlayerController* PC) const;

	void UpdateTargetCamera(float DeltaTime);
	void ApplyControlRotation(FRotator NewRotation, float DeltaTime);
	void CheckTargetLoss();
};