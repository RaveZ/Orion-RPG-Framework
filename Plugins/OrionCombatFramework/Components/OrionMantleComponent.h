// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "../Character/OrionCharacterBase.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"

#include "OrionMantleComponent.generated.h"

// forward declarations
class UOrionDebugComponent;


UCLASS(Blueprintable, BlueprintType)
class ORIONCOMBATFRAMEWORK_API UOrionMantleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOrionMantleComponent();

	UFUNCTION(BlueprintCallable, Category = "Orion|Mantle System")
	bool MantleCheck(const FOrionMantleTraceSettings& TraceSettings,
	                 EDrawDebugTrace::Type DebugType);

	UFUNCTION(BlueprintCallable, Category = "Orion|Mantle System")
	void MantleStart(float MantleHeight, const FOrionComponentAndTransform& MantleLedgeWS,
	                EOrionMantleType MantleType);

	UFUNCTION(BlueprintCallable, Category = "Orion|Mantle System")
	void MantleUpdate(float BlendIn);

	UFUNCTION(BlueprintCallable, Category = "Orion|Mantle System")
	void MantleEnd();

	UFUNCTION(BlueprintCallable, Category = "Orion|Mantle System")
	void OnOwnerJumpInput();

	UFUNCTION(BlueprintCallable, Category = "Orion|Mantle System")
	void OnOwnerRagdollStateChanged(bool bRagdollState);

	/** Implement on BP to get correct mantle parameter set according to character state */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Orion|Mantle System")
	FOrionMantleAsset GetMantleAsset(EOrionMantleType MantleType, EOrionOverlayState CurrentOverlayState);

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	// Called when the game starts
	virtual void BeginPlay() override;

	/** Mantling*/
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Orion|Mantle System")
	void Server_MantleStart(float MantleHeight, const FOrionComponentAndTransform& MantleLedgeWS,
	                        EOrionMantleType MantleType);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Orion|Mantle System")
	void Multicast_MantleStart(float MantleHeight, const FOrionComponentAndTransform& MantleLedgeWS,
	                           EOrionMantleType MantleType);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orion|Mantle System")
	TObjectPtr<UTimelineComponent> MantleTimeline = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Orion|Mantle System")
	FOrionMantleTraceSettings GroundedTraceSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Orion|Mantle System")
	FOrionMantleTraceSettings AutomaticTraceSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Orion|Mantle System")
	FOrionMantleTraceSettings FallingTraceSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Orion|Mantle System")
	TObjectPtr<UCurveFloat> MantleTimelineCurve;

	static FName NAME_IgnoreOnlyPawn;
	/** Profile to use to detect objects we allow mantling */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Orion|Mantle System")
	FName MantleObjectDetectionProfile = NAME_IgnoreOnlyPawn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Orion|Mantle System")
	TEnumAsByte<ECollisionChannel> WalkableSurfaceDetectionChannel = ECC_Visibility;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Mantle System")
	FOrionMantleParams MantleParams;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Mantle System")
	FOrionComponentAndTransform MantleLedgeLS;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Mantle System")
	FTransform MantleTarget = FTransform::Identity;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Mantle System")
	FTransform MantleActualStartOffset = FTransform::Identity;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Mantle System")
	FTransform MantleAnimatedStartOffset = FTransform::Identity;

	/** If a dynamic object has a velocity bigger than this value, do not start mantle */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Orion|Mantle System")
	float AcceptableVelocityWhileMantling = 10.0f;

private:
	UPROPERTY()
	TObjectPtr<AOrionCharacterBase> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UOrionDebugComponent> OrionDebugComponent = nullptr;
};
