// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OrionCombatFramework/OrionCombatFramework.h"
#include "../Ability/OrionGameplayAbility.h"
#include "OrionProjectileSpell.generated.h"

class UGameplayEffect;
class AOrionProjectile;

UCLASS()
class ORIONCOMBATFRAMEWORK_API UOrionProjectileSpell : public UOrionGameplayAbility
{
	GENERATED_BODY()
protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectile(const FVector& ProjectileTargetLocation);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AOrionProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;
};
