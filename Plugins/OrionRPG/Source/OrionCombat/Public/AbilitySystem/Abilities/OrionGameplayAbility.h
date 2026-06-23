// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "OrionGameplayAbility.generated.h"


class APlayerController;
class UOrionAbilitySystemComponent;

/**
 * EOrionAbilityActivationPolicy
 *
 *	Defines how an ability is meant to activate.
 */
UENUM(BlueprintType)
enum class EOrionAbilityActivationPolicy : uint8
{
	// Try to activate the ability when the input is triggered.
	OnInputTriggered,

	// Continually try to activate the ability while the input is active.
	WhileInputActive,
};

/**
 * UOrionGameplayAbility
 *
 *	The base gameplay ability class used by this project.
 */
UCLASS(Abstract, HideCategories = Input, Meta = (ShortTooltip = "The base gameplay ability class used by this project."))
class ORIONCOMBAT_API UOrionGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UOrionGameplayAbility();

	UFUNCTION(BlueprintCallable, Category = "Orion|Ability")
	UOrionAbilitySystemComponent* GetOrionAbilitySystemComponentFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Orion|Ability")
	APlayerController* GetPlayerControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Orion|Ability")
	AController* GetControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Orion|Ability")
	ACharacter* GetCharacterFromActorInfo() const;
	
protected:
	// Defines how this ability is meant to activate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Ability Activation")
	EOrionAbilityActivationPolicy ActivationPolicy;
	
public:
	EOrionAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }

protected:
	//~UGameplayAbility interface
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void SetCanBeCanceled(bool bCanBeCanceled) override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~End of UGameplayAbility interface
	/** Called when this ability is granted to the ability system component. */

	virtual void OnPawnAvatarSet();

	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityAdded")
	void K2_OnAbilityAdded();

	/** Called when this ability is removed from the ability system component. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityRemoved")
	void K2_OnAbilityRemoved();

	/** Called when the ability system is initialized with a pawn avatar. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnPawnAvatarSet")
	void K2_OnPawnAvatarSet();

};
