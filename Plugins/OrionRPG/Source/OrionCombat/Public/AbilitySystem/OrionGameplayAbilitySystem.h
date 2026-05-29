// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "OrionGameplayAbilitySystem.generated.h"

/**
 * Base ability system component for Orion combat module.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "OrionCombat", meta = (BlueprintSpawnableComponent), DisplayName = "Orion Gameplay Ability System")
class ORIONCOMBAT_API UOrionGameplayAbilitySystem : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UOrionGameplayAbilitySystem();
};
