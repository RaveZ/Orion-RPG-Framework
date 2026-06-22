// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Animation/AnimNotify/OrionAnimNotifyState_DamageTrace.h"

#include "OrionCombatFunctionLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/OrionAbilitySystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Character.h"



UOrionAnimNotifyState_DamageTrace::UOrionAnimNotifyState_DamageTrace()
{
	TraceInterval = 0.033333f;
	TraceRadius = 32.0f;
	bDrawDebug = true;
	HitResults.Reserve(4);
	TraceTimer = 0.0f;

#if WITH_EDITORONLY_DATA
	NotifyColor = FColor::Red;
#endif // WITH_EDITORONLY_DATA
}


FString UOrionAnimNotifyState_DamageTrace::GetNotifyName_Implementation() const
{
	return TEXT("ANS_DamageSphereTrace");
}


void UOrionAnimNotifyState_DamageTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	//UE_LOG(LogTemp, Log, TEXT("ANS_DamageSphereTrace: Begin!"));

	CachedHitActors.Reset();
	HitResults.Reset();
	TraceTimer = 0.0f;

	PerformTrace(MeshComp);
}


void UOrionAnimNotifyState_DamageTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
	//UE_LOG(LogTemp, Log, TEXT("ANS_DamageSphereTrace: Tick %.3f"), FrameDeltaTime);

	TraceTimer += FrameDeltaTime;
	
	if (TraceTimer >= TraceInterval)
	{
		TraceTimer -= TraceInterval;
		PerformTrace(MeshComp);
	}
}


void UOrionAnimNotifyState_DamageTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	//UE_LOG(LogTemp, Log, TEXT("ANS_DamageSphereTrace: End!"));

	CachedHitActors.Reset();
	HitResults.Reset();
	TraceTimer = 0.0f;
}


void UOrionAnimNotifyState_DamageTrace::PerformTrace(USkeletalMeshComponent* SkelMeshComp)
{
	if (SkelMeshComp == nullptr)
	{
		return;
	}

	//UE_LOG(LogTemp, Log, TEXT("PerformTrace"));

	ACharacter* CharOwner = Cast<ACharacter>(SkelMeshComp->GetOwner());
	if (CharOwner == nullptr)
	{
		return;
	}

	static FName TraceTagName = "ANS_DamageSphereTrace";

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CharOwner);
	QueryParams.TraceTag = TraceTagName;

	const FVector TraceLocation = SkelMeshComp->GetSocketLocation(TraceSocketName);

#if WITH_EDITOR
	if (bDrawDebug)
	{
		UKismetSystemLibrary::DrawDebugCircle(SkelMeshComp, TraceLocation, TraceRadius, 16, FLinearColor::Red, 1.0f, 1.0f);
	}
#endif // WITH_EDITOR

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(CharOwner);

	UKismetSystemLibrary::SphereTraceMulti(SkelMeshComp, TraceLocation, TraceLocation, TraceRadius, TraceChannel, false, IgnoredActors, EDrawDebugTrace::ForDuration, HitResults, true, FLinearColor::Red, FLinearColor::Green, 1.0f);

	for (int HitIndex = 0; HitIndex < HitResults.Num(); ++HitIndex)
	{
		AActor* HitActor = HitResults[HitIndex].GetActor();
		if (HitActor == nullptr)
		{
			continue;
		}

		bool bFound = false;
		for (int32 ActorIndex = 0; ActorIndex < CachedHitActors.Num(); ++ActorIndex)
		{
			if (CachedHitActors[ActorIndex] == HitActor)
			{
				bFound = true;
				break;
			}
		}

		// Ignore already hit actor
		if (bFound)
		{
			//UE_LOG(LogTemp, Log, TEXT("ANS_DamageSphereTrace: Already hit!"));
			continue;
		}

		UOrionAbilitySystemComponent* HitASC = UOrionCombatFunctionLibrary::GetOrionAbilityComponentFromTarget(HitActor);
		if (HitASC == nullptr || (!RequiredTags.IsEmpty() && !HitASC->HasAllMatchingGameplayTags(RequiredTags)))
		{
			UE_LOG(LogTemp, Log, TEXT("ANS_DamageSphereTrace: Got hit actor but ASC is not valid or RequiredTags check failed!"));
			continue;
		}

		UE_LOG(LogTemp, Log, TEXT("ANS_DamageSphereTrace: Hit actor [%s]"), *HitActor->GetName());

		// Execute damage ability by event
		HitActor->ActivateAbilityByEvent(DamageEventTag, DamageAbilityClass, CharOwner, CharOwner);

		// Apply gameplay effects
		HitActor->ApplyEffects(DamageEffectClasses, CharOwner, CharOwner);

		CachedHitActors.Add(HitActor);
	}
}
