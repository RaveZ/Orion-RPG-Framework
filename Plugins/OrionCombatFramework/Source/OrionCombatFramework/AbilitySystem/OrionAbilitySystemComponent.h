// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"

#include "OrionAbilitySystemComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer&/*Asset Tags*/)

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ORIONCOMBATFRAMEWORK_API UOrionAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UOrionAbilitySystemComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;
	void AbilityActorInfoSet();

	FEffectAssetTags EffectAssetTags;

	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);

	void AbilityInputTagHeld(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
protected:
	virtual void BeginPlay() override;

	UFUNCTION(Client, Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle);
};
