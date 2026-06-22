// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "OrionAnimNotifyState_DamageTrace.generated.h"

class UOrionGameplayAbility;
/**
 * 
 */
UCLASS()
class ORIONCOMBAT_API UOrionAnimNotifyState_DamageTrace : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	FName TraceSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float TraceInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	TEnumAsByte<ETraceTypeQuery> TraceChannel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float TraceRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	bool bDrawDebug;

	// Hit actor must have these tags
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
	FGameplayTag DamageEventTag;

	// Gameplay ability to execute on each hit actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
	TSubclassOf<UOrionGameplayAbility> DamageAbilityClass;

	// Gameplay effects to apply on each hit actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
	TArray<TSubclassOf<UGameplayEffect>> DamageEffectClasses;

	UPROPERTY()
	TArray<AActor*> CachedHitActors;

	TArray<FHitResult> HitResults;

	float TraceTimer;


public:
	UDHAnimNotifyState_DamageSphereTrace();
	virtual FString GetNotifyName_Implementation() const override;
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

private:
	void PerformTrace(USkeletalMeshComponent* SkelMeshComp);

};
