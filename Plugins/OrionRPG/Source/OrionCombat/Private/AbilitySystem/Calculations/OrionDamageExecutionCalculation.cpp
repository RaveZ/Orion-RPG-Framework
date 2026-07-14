// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "AbilitySystem/Calculations/OrionDamageExecutionCalculation.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/Attributes/OrionAttributeSetBase.h"
#include "GameplayEffect.h"
#include "GameplayEffectExecutionCalculation.h"
#include "OrionCombatData.h"

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
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	if (!TargetASC) return;
	
	AActor* TargetActor = TargetASC->GetOwner();
	if (!TargetActor) return;
	
	FGameplayTagContainer TargetTags;
	TargetASC->GetOwnedGameplayTags(TargetTags);

	
    
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
    
	
	FGameplayEffectContext* TypedContext = Spec.GetContext().Get();
	check(TypedContext);
	
	const FHitResult* HitActorResult = TypedContext->GetHitResult();
	
	
	float IncomingDamage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		OrionDamageStatics().DamageDef, EvaluateParameters, IncomingDamage);
	/*IncomingDamage = FMath::Max(IncomingDamage, 
		Spec.GetSetByCallerMagnitude(TAG_Gameplay_Damage, false, 0.0f));
		*/
	
	
	float Defense = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		OrionDamageStatics().DefenseDef, EvaluateParameters, Defense);
	
	float FinalDamage = FMath::Max(0.0f, IncomingDamage - Defense);
	
	//Parry Calculation: Full Damage Negation
	if (TargetTags.HasTag(TAG_GameplayEffect_Defense_Parry))
	{
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(
				UOrionAttributeSetBase::GetDamageAttribute(),
				EGameplayModOp::Additive,
				0.0f
			)
		);
		
		{//Send Gameplay Event Parried
			FGameplayEventData Payload;          
			Payload.EventTag = TAG_Event_Defense_Parried;
			Payload.Instigator = Spec.GetContext().GetInstigator();
			Payload.Target = TargetActor;
			Payload.OptionalObject = Spec.GetContext().GetSourceObject();
			Payload.OptionalObject2 = Spec.GetContext().GetEffectCauser();
			Payload.ContextHandle = Spec.GetContext();
			Payload.InstigatorTags = *Spec.CapturedSourceTags.GetAggregatedTags();
			Payload.TargetTags = *Spec.CapturedTargetTags.GetAggregatedTags();
			Payload.EventMagnitude = FinalDamage;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, TAG_Event_Defense_Parried, Payload);
		}
		return;
	}
	
	if (TargetTags.HasTag(TAG_GameplayEffect_Defense_Block))
	{
		FinalDamage = FinalDamage * 0.5f;/*Block Coefficient*/
		
		{//Send Gameplay Event Blocked
			FGameplayEventData Payload;          
			Payload.EventTag = TAG_Event_Defense_Blocked;
			Payload.Instigator = Spec.GetContext().GetInstigator();
			Payload.Target = TargetActor;
			Payload.OptionalObject = Spec.GetContext().GetSourceObject();
			Payload.OptionalObject2 = Spec.GetContext().GetEffectCauser();
			Payload.ContextHandle = Spec.GetContext();
			Payload.InstigatorTags = *Spec.CapturedSourceTags.GetAggregatedTags();
			Payload.TargetTags = *Spec.CapturedTargetTags.GetAggregatedTags();
			Payload.EventMagnitude = FinalDamage;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, TAG_Event_Defense_Blocked, Payload);
		}
	}
    
    
	// Output to meta attribute instead of directly to Health
	OutExecutionOutput.AddOutputModifier(
		FGameplayModifierEvaluatedData(
			UOrionAttributeSetBase::GetDamageAttribute(),  // Meta attribute
			EGameplayModOp::Additive,  
			FinalDamage
		)
	);
}
