// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "../Ability/OrionProjectileSpell.h"
#include "../../Actor/OrionProjectile.h"
#include "../../Interaction/CombatInterface.h"
#include "../../OrionGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"


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
        UE_LOG(OrionLog, Warning, TEXT("OrionProjectile : ProjectileClass is NULL!"));
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
        const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), SourceASC->MakeEffectContext());
        Projectile->DamageEffectSpecHandle = SpecHandle;

        Projectile->FinishSpawning(SpawnTransform);
    }
}
