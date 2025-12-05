// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "ActiveGameplayEffectHandle.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayAbilitySpecHandle.h"
#include "Templates/SubclassOf.h"

#include "OrionGlobalAbilitySystem.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UOrionAbilitySystemComponent;
class UObject;
struct FActiveGameplayEffectHandle;
struct FFrame;
struct FGameplayAbilitySpecHandle;

USTRUCT()
struct FOrionGlobalAppliedAbilityList
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TObjectPtr<UOrionAbilitySystemComponent>, FGameplayAbilitySpecHandle> Handles;

	void AddToASC(TSubclassOf<UGameplayAbility> Ability, UOrionAbilitySystemComponent* ASC);
	void RemoveFromASC(UOrionAbilitySystemComponent* ASC);
	void RemoveFromAll();
};

USTRUCT()
struct FOrionGlobalAppliedEffectList
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TObjectPtr<UOrionAbilitySystemComponent>, FActiveGameplayEffectHandle> Handles;

	void AddToASC(TSubclassOf<UGameplayEffect> Effect, UOrionAbilitySystemComponent* ASC);
	void RemoveFromASC(UOrionAbilitySystemComponent* ASC);
	void RemoveFromAll();
};

UCLASS()
class ORIONRPG_API UOrionGlobalAbilitySystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UOrionGlobalAbilitySystem();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Orion")
	void ApplyAbilityToAll(TSubclassOf<UGameplayAbility> Ability);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Orion")
	void ApplyEffectToAll(TSubclassOf<UGameplayEffect> Effect);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Orion")
	void RemoveAbilityFromAll(TSubclassOf<UGameplayAbility> Ability);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Orion")
	void RemoveEffectFromAll(TSubclassOf<UGameplayEffect> Effect);

	/** Register an ASC with global system and apply any active global effects/abilities. */
	void RegisterASC(UOrionAbilitySystemComponent* ASC);

	/** Removes an ASC from the global system, along with any active global effects/abilities. */
	void UnregisterASC(UOrionAbilitySystemComponent* ASC);

private:
	UPROPERTY()
	TMap<TSubclassOf<UGameplayAbility>, FOrionGlobalAppliedAbilityList> AppliedAbilities;

	UPROPERTY()
	TMap<TSubclassOf<UGameplayEffect>, FOrionGlobalAppliedEffectList> AppliedEffects;

	UPROPERTY()
	TArray<TObjectPtr<UOrionAbilitySystemComponent>> RegisteredASCs;
};
