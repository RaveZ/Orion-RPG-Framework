// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "AbilitySystem/OrionGameplayAbilitySystem.h"
#include "GameplayEffectTypes.h"

UOrionGameplayAbilitySystem::UOrionGameplayAbilitySystem()
{
	SetIsReplicatedByDefault(true);
	SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}
