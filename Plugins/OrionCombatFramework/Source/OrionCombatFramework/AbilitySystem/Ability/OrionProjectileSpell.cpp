// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "../Ability/OrionProjectileSpell.h"
#include "../../Actor/OrionProjectile.h"
#include "../../Interaction/CombatInterface.h"
#include "../../OrionGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "../../OrionLogChannels.h"


void UOrionProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    
}

void UOrionProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation)
{
    const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
    if (!bIsServer) return;

    if (ProjectileClass == nullptr)
    {
        UE_LOG(LogOrion, Warning, TEXT("OrionProjectile : ProjectileClass is NULL!"));
        return;
    }

    ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
    if (CombatInterface)
    {
        const FVector SocketLocation = CombatInterface->GetCombatSocketLocation();
        FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
        Rotation.Pitch = 0.f;

        FTransform SpawnTransform;
        SpawnTransform.SetLocation(SocketLocation);
        SpawnTransform.SetRotation(Rotation.Quaternion());

        AOrionProjectile* Projectile = GetWorld()->SpawnActorDeferred<AOrionProjectile>(
            ProjectileClass,
            SpawnTransform,
            GetOwningActorFromActorInfo(),
            Cast<APawn>(GetOwningActorFromActorInfo()),
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

        const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
		FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
		EffectContextHandle.SetAbility(this);
		EffectContextHandle.AddSourceObject(Projectile);
        TArray<TWeakObjectPtr<AActor>> Actors;
        Actors.Add(Projectile);
		EffectContextHandle.AddActors(Actors);
		FHitResult HitResult;
        HitResult.Location = ProjectileTargetLocation;
		EffectContextHandle.AddHitResult(HitResult);

        const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContextHandle);

        //TODO : make variable level so it can be used modular
        const float ScaledDamage = Damage.GetValueAtLevel(5);

        UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, TAG_Damage, ScaledDamage);
        Projectile->DamageEffectSpecHandle = SpecHandle;

        Projectile->FinishSpawning(SpawnTransform);
    }
}
