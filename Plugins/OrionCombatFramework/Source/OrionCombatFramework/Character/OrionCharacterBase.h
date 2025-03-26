// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "OrionCombatFramework/Interaction/CombatInterface.h"
#include "ScalableFloat.h"
#include "../Progression/Reward.h"
#include "OrionCharacterBase.generated.h"

class UAnimMontage;
class UGameplayEffect;
class UAttributeSet;
class UAbilitySystemComponent;
class UGameplayAbility;



UCLASS(Abstract)
class ORIONCOMBATFRAMEWORK_API AOrionCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	AOrionCharacterBase();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const {return AttributeSet;}

	virtual UAnimMontage* GetHitReactMontage_Implementation() override;

	virtual void Die() override;

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath();
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category="Orion | Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

	UPROPERTY(EditAnywhere, Category = "Orion | Combat")
	FName WeaponTipSocketName;

	virtual FVector GetCombatSocketLocation() override;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	virtual void InitAbilityActorInfo();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Orion | Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Orion | Attributes")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Orion | Attributes")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;

	UPROPERTY(EditDefaultsOnly, Category = "Orion | Progression")
	FScalableFloat XPReward = FScalableFloat();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Orion | Progression")
	TArray<FRewardInfo> DefeatReward;

	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;
	
	void InitializeDefaultAttributes() const;

	void AddCharacterAbilities();

	/* Dissolve Effects */

	void Dissolve();

	UFUNCTION(BlueprintImplementableEvent)
	void StartDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);

	UFUNCTION(BlueprintImplementableEvent)
	void StartWeaponDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstance;
private:

	UPROPERTY(EditAnywhere, Category = "Orion | Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
	
	UPROPERTY(EditAnywhere, Category = "Orion | Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupPassiveAbilities;

	UPROPERTY(EditAnywhere, Category = "Orion |Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;

public:
	FORCEINLINE FScalableFloat GetXPReward() const { return XPReward; }
};