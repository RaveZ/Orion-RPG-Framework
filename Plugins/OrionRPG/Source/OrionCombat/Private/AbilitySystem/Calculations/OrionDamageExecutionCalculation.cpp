// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "AbilitySystem/Calculations/OrionDamageExecutionCalculation.h"

#include "AbilitySystem/Attributes/OrionAttributeSetBase.h"
#include "GameplayEffect.h"
#include "GameplayEffectExecutionCalculation.h"

struct FOrionDamageStatics
{
	FGameplayEffectAttributeCaptureDefinition DamageDef;
	FGameplayEffectAttributeCaptureDefinition DefenseDef;

	FOrionDamageStatics()
	{
		DamageDef = FGameplayEffectAttributeCaptureDefinition(UOrionAttributeSetBase::GetDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
		DefenseDef = FGameplayEffectAttributeCaptureDefinition(UOrionAttributeSetBase::GetDefenseAttribute(), EGameplayEffectAttributeCaptureSource::Target, false);
	}
};

static const FOrionDamageStatics& OrionDamageStatics()
{
	static FOrionDamageStatics Statics;
	return Statics;
}

UOrionDamageExecutionCalculation::UOrionDamageExecutionCalculation()
{
	RelevantAttributesToCapture.Add(OrionDamageStatics().DamageDef);
	RelevantAttributesToCapture.Add(OrionDamageStatics().DefenseDef);
}

void UOrionDamageExecutionCalculation::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
    
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
    
	
	FGameplayEffectContext* TypedContext = Spec.GetContext().Get();
	check(TypedContext);
	
	const FHitResult* HitActorResult = TypedContext->GetHitResult();

	AActor* HitActor = nullptr;
	FVector ImpactLocation = FVector::ZeroVector;
	FVector ImpactNormal = FVector::ZeroVector;
	FVector StartTrace = FVector::ZeroVector;
	FVector EndTrace = FVector::ZeroVector;

	// Calculation of hit actor, surface, zone, and distance all rely on whether the calculation has a hit result or not.
	// Effects just being added directly w/o having been targeted will always come in without a hit result, which must default
	// to some fallback information.
	if (HitActorResult)
	{
		const FHitResult& CurHitResult = *HitActorResult;
		HitActor = CurHitResult.HitObjectHandle.FetchActor();
		if (HitActor)
		{
			ImpactLocation = CurHitResult.ImpactPoint;
			ImpactNormal = CurHitResult.ImpactNormal;
			StartTrace = CurHitResult.TraceStart;
			EndTrace = CurHitResult.TraceEnd;
		}
	}
	
	float IncomingDamage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		OrionDamageStatics().DamageDef, EvaluateParameters, IncomingDamage);
	IncomingDamage = FMath::Max(IncomingDamage, 
		Spec.GetSetByCallerMagnitude(TAG_Gameplay_Damage, false, 0.0f));
    
	float Defense = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		OrionDamageStatics().DefenseDef, EvaluateParameters, Defense);
    
	const float FinalDamage = FMath::Max(0.0f, IncomingDamage - Defense);
    
	// Output to meta attribute instead of directly to Health
	OutExecutionOutput.AddOutputModifier(
		FGameplayModifierEvaluatedData(
			UOrionAttributeSetBase::GetDamageAttribute(),  // Meta attribute
			EGameplayModOp::Additive,  
			FinalDamage
		)
	);
}
