// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "OrionPlayerCameraManager.generated.h"

// forward declarations
class UOrionDebugComponent;
class AOrionCharacterBase;

/**
 * Player camera manager class
 */
UCLASS(Blueprintable, BlueprintType)
class ORIONCOMBATFRAMEWORK_API AOrionPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	AOrionPlayerCameraManager();

	UFUNCTION(BlueprintCallable, Category = "Orion|Camera")
	void OnPossess(AOrionCharacterBase* NewCharacter);

	UFUNCTION(BlueprintCallable, Category = "Orion|Camera")
	float GetCameraBehaviorParam(FName CurveName) const;

	/** Implemented debug logic in BP */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Orion|Camera")
	void DrawDebugTargets(FVector PivotTargetLocation);

protected:
	virtual void UpdateViewTargetInternal(FTViewTarget& OutVT, float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Orion|Camera")
	static FVector CalculateAxisIndependentLag(
		FVector CurrentLocation, FVector TargetLocation, FRotator CameraRotation, FVector LagSpeeds, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Orion|Camera")
	bool CustomCameraBehavior(float DeltaTime, FVector& Location, FRotator& Rotation, float& FOV);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Orion|Camera")
	TObjectPtr<AOrionCharacterBase> ControlledCharacter = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Orion|Camera")
	TObjectPtr<USkeletalMeshComponent> CameraBehavior = nullptr;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Orion|Camera")
	FVector RootLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Orion|Camera")
	FTransform SmoothedPivotTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Orion|Camera")
	FVector PivotLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Orion|Camera")
	FVector TargetCameraLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Orion|Camera")
	FRotator TargetCameraRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Orion|Camera")
	FRotator DebugViewRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Orion|Camera")
	FVector DebugViewOffset;

private:
	UPROPERTY()
	TObjectPtr<UOrionDebugComponent> OrionDebugComponent = nullptr;
};
