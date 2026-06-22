// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "OrionAnimNotifyState_DamageTrace.generated.h"

class UGameplayEffect;
class UOrionGameplayAbility;

UENUM(BlueprintType)
enum class EOrionDamageTraceShape : uint8
{
	Sphere,
	Box,
	Capsule,
	Line
};

/**
 * 
 */
UCLASS(Category = "Orion", DisplayName = "Orion - Damage Trace", AutoExpandCategories = "Trace, Trace|Shape, Abilities")
class ORIONCOMBAT_API UOrionAnimNotifyState_DamageTrace : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace", meta = (AnimNotifyBoneName = "true"))
	FName TraceSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	FVector TraceOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	FRotator TraceRotationOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float TraceInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	TEnumAsByte<ETraceTypeQuery> TraceChannel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace|Shape")
	EOrionDamageTraceShape TraceShape;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace|Shape", meta = (EditCondition = "TraceShape == EOrionDamageTraceShape::Sphere", EditConditionHides, ClampMin = "0.0"))
	float SphereRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace|Shape", meta = (EditCondition = "TraceShape == EOrionDamageTraceShape::Box", EditConditionHides))
	FVector BoxHalfExtent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace|Shape", meta = (EditCondition = "TraceShape == EOrionDamageTraceShape::Capsule", EditConditionHides, ClampMin = "0.0"))
	float CapsuleRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace|Shape", meta = (EditCondition = "TraceShape == EOrionDamageTraceShape::Capsule", EditConditionHides, ClampMin = "0.0"))
	float CapsuleHalfHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace|Shape", meta = (EditCondition = "TraceShape == EOrionDamageTraceShape::Line", EditConditionHides, ClampMin = "0.0"))
	float LineLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	bool bDrawDebug;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace", meta = (EditCondition = "bDrawDebug", EditConditionHides, ClampMin = "0.0"))
	float DebugDrawDuration;

	// Hit actor must have these tags
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	FGameplayTag DamageEventTag;

	// Gameplay ability to execute on each hit actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TSubclassOf<UOrionGameplayAbility> DamageAbilityClass;

	// Gameplay effects to apply on each hit actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TArray<TSubclassOf<UGameplayEffect>> DamageEffectClasses;

	UPROPERTY()
	TArray<AActor*> CachedHitActors;

	TArray<FHitResult> HitResults;

	float TraceTimer;


public:
	UOrionAnimNotifyState_DamageTrace();
	virtual FString GetNotifyName_Implementation() const override;
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

private:
	void PerformTrace(USkeletalMeshComponent* SkelMeshComp);
	void ProcessHit(const FHitResult& HitResult, AActor* SourceActor);
	void TriggerDamageAbility(class UOrionAbilitySystemComponent* HitASC, AActor* HitActor, AActor* SourceActor, const FHitResult& HitResult) const;
	void ApplyDamageEffects(class UOrionAbilitySystemComponent* SourceASC, class UOrionAbilitySystemComponent* HitASC, const FHitResult& HitResult) const;
	FTransform GetTraceTransform(const USkeletalMeshComponent* SkelMeshComp) const;
	bool IsGameplayWorld(const UWorld* World) const;
	void DrawDebugTrace(const UWorld* World, const FTransform& TraceTransform, const FVector& LineEnd) const;

};
