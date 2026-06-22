// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Animation/AnimNotify/OrionAnimNotifyState_DamageTrace.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/Abilities/OrionGameplayAbility.h"
#include "AbilitySystem/OrionAbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "GameplayEffect.h"
#include "OrionCombatFunctionLibrary.h"

UOrionAnimNotifyState_DamageTrace::UOrionAnimNotifyState_DamageTrace()
{
	TraceSocketName = NAME_None;
	TraceOffset = FVector::ZeroVector;
	TraceRotationOffset = FRotator::ZeroRotator;
	TraceInterval = 0.033333f;
	TraceShape = EOrionDamageTraceShape::Sphere;
	SphereRadius = 32.0f;
	BoxHalfExtent = FVector(32.0f);
	CapsuleRadius = 32.0f;
	CapsuleHalfHeight = 64.0f;
	LineLength = 100.0f;
	bDrawDebug = true;
	DebugDrawDuration = 1.0f;
	HitResults.Reserve(4);
	TraceTimer = 0.0f;

#if WITH_EDITORONLY_DATA
	NotifyColor = FColor::Red;
#endif
}

FString UOrionAnimNotifyState_DamageTrace::GetNotifyName_Implementation() const
{
	return TEXT("ANS_DamageTrace");
}

void UOrionAnimNotifyState_DamageTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	CachedHitActors.Reset();
	HitResults.Reset();
	TraceTimer = 0.0f;

	PerformTrace(MeshComp);
}

void UOrionAnimNotifyState_DamageTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
	TraceTimer += FrameDeltaTime;

	if (TraceTimer >= TraceInterval)
	{
		TraceTimer -= TraceInterval;
		PerformTrace(MeshComp);
	}
}

void UOrionAnimNotifyState_DamageTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	CachedHitActors.Reset();
	HitResults.Reset();
	TraceTimer = 0.0f;
}

FTransform UOrionAnimNotifyState_DamageTrace::GetTraceTransform(const USkeletalMeshComponent* SkelMeshComp) const
{
	const FTransform SocketTransform = SkelMeshComp->GetSocketTransform(TraceSocketName, RTS_World);
	const FTransform OffsetTransform(TraceRotationOffset, TraceOffset);
	return OffsetTransform * SocketTransform;
}

bool UOrionAnimNotifyState_DamageTrace::IsGameplayWorld(const UWorld* World) const
{
	return World && (World->IsGameWorld() || World->WorldType == EWorldType::PIE);
}

void UOrionAnimNotifyState_DamageTrace::DrawDebugTrace(const UWorld* World, const FTransform& TraceTransform, const FVector& LineEnd) const
{
	if (!bDrawDebug || !World)
	{
		return;
	}

	const FVector Location = TraceTransform.GetLocation();
	const FQuat Rotation = TraceTransform.GetRotation();

	switch (TraceShape)
	{
	case EOrionDamageTraceShape::Sphere:
		DrawDebugSphere(World, Location, SphereRadius, 16, FColor::Red, false, DebugDrawDuration);
		break;

	case EOrionDamageTraceShape::Box:
		DrawDebugBox(World, Location, BoxHalfExtent, Rotation, FColor::Red, false, DebugDrawDuration);
		break;

	case EOrionDamageTraceShape::Capsule:
		DrawDebugCapsule(World, Location, CapsuleHalfHeight, CapsuleRadius, Rotation, FColor::Red, false, DebugDrawDuration);
		break;

	case EOrionDamageTraceShape::Line:
		DrawDebugLine(World, Location, LineEnd, FColor::Red, false, DebugDrawDuration, 0, 1.5f);
		break;
	}
}

void UOrionAnimNotifyState_DamageTrace::PerformTrace(USkeletalMeshComponent* SkelMeshComp)
{
	if (!SkelMeshComp)
	{
		return;
	}

	UWorld* World = SkelMeshComp->GetWorld();
	const FTransform TraceTransform = GetTraceTransform(SkelMeshComp);
	const FVector TraceStart = TraceTransform.GetLocation();
	const FVector TraceEnd = TraceShape == EOrionDamageTraceShape::Line
		? TraceStart + TraceTransform.GetRotation().GetForwardVector() * LineLength
		: TraceStart;

	DrawDebugTrace(World, TraceTransform, TraceEnd);

	AActor* SourceActor = SkelMeshComp->GetOwner();
	HitResults.Reset();

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(OrionDamageTrace), false, SourceActor);
	QueryParams.bReturnPhysicalMaterial = true;

	const ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceChannel);
	const FQuat TraceRotation = TraceTransform.GetRotation();

	switch (TraceShape)
	{
	case EOrionDamageTraceShape::Sphere:
		World->SweepMultiByChannel(HitResults, TraceStart, TraceEnd, TraceRotation, CollisionChannel, FCollisionShape::MakeSphere(SphereRadius), QueryParams);
		break;

	case EOrionDamageTraceShape::Box:
		World->SweepMultiByChannel(HitResults, TraceStart, TraceEnd, TraceRotation, CollisionChannel, FCollisionShape::MakeBox(BoxHalfExtent), QueryParams);
		break;

	case EOrionDamageTraceShape::Capsule:
		World->SweepMultiByChannel(HitResults, TraceStart, TraceEnd, TraceRotation, CollisionChannel, FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight), QueryParams);
		break;

	case EOrionDamageTraceShape::Line:
		World->LineTraceMultiByChannel(HitResults, TraceStart, TraceEnd, CollisionChannel, QueryParams);
		break;
	}

	if (!IsGameplayWorld(World))
	{
		return;
	}

	for (const FHitResult& HitResult : HitResults)
	{
		ProcessHit(HitResult, SourceActor);
	}
}

void UOrionAnimNotifyState_DamageTrace::ProcessHit(const FHitResult& HitResult, AActor* SourceActor)
{
	AActor* HitActor = HitResult.GetActor();
	if (!HitActor || HitActor == SourceActor)
	{
		return;
	}

	if (CachedHitActors.Contains(HitActor))
	{
		return;
	}

	UOrionAbilitySystemComponent* HitASC = UOrionCombatFunctionLibrary::GetOrionAbilityComponentFromTarget(HitActor);
	if (!HitASC || (!RequiredTags.IsEmpty() && !HitASC->HasAllMatchingGameplayTags(RequiredTags)))
	{
		UE_LOG(LogTemp, Verbose, TEXT("ANS_DamageTrace: Hit actor [%s] failed ASC or RequiredTags check."), *GetNameSafe(HitActor));
		return;
	}

	UOrionAbilitySystemComponent* SourceASC = UOrionCombatFunctionLibrary::GetOrionAbilityComponentFromTarget(SourceActor);

	TriggerDamageAbility(HitASC, HitActor, SourceActor, HitResult);
	ApplyDamageEffects(SourceASC, HitASC, HitResult);

	CachedHitActors.Add(HitActor);
}

void UOrionAnimNotifyState_DamageTrace::TriggerDamageAbility(UOrionAbilitySystemComponent* HitASC, AActor* HitActor, AActor* SourceActor, const FHitResult& HitResult) const
{
	if (!HitASC || !DamageAbilityClass || !DamageEventTag.IsValid())
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = DamageEventTag;
	Payload.Instigator = SourceActor;
	Payload.Target = HitActor;
	Payload.ContextHandle = HitASC->MakeEffectContext();
	Payload.ContextHandle.AddInstigator(SourceActor, SourceActor);
	Payload.ContextHandle.AddHitResult(HitResult);

	TArray<FGameplayAbilitySpecHandle> MatchingHandles;
	for (const FGameplayAbilitySpec& Spec : HitASC->GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability->GetClass()->IsChildOf(DamageAbilityClass))
		{
			MatchingHandles.Add(Spec.Handle);
		}
	}

	for (const FGameplayAbilitySpecHandle& Handle : MatchingHandles)
	{
		HitASC->TriggerAbilityFromGameplayEvent(Handle, HitASC->AbilityActorInfo.Get(), DamageEventTag, &Payload, *HitASC);
	}
}

void UOrionAnimNotifyState_DamageTrace::ApplyDamageEffects(UOrionAbilitySystemComponent* SourceASC, UOrionAbilitySystemComponent* HitASC, const FHitResult& HitResult) const
{
	if (!HitASC)
	{
		return;
	}

	UAbilitySystemComponent* SpecSourceASC = SourceASC ? SourceASC : HitASC;
	for (const TSubclassOf<UGameplayEffect>& EffectClass : DamageEffectClasses)
	{
		if (!EffectClass)
		{
			continue;
		}

		FGameplayEffectContextHandle Context = SpecSourceASC->MakeEffectContext();
		Context.AddInstigator(SourceASC ? SourceASC->GetOwnerActor() : nullptr, SourceASC ? SourceASC->GetAvatarActor() : nullptr);
		Context.AddHitResult(HitResult);

		FGameplayEffectSpecHandle SpecHandle = SpecSourceASC->MakeOutgoingSpec(EffectClass, 1.0f, Context);
		if (SpecHandle.IsValid())
		{
			SpecSourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), HitASC);
		}
	}
}
