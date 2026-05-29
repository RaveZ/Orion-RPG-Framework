// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionCombatAssetManager.h"
#include "AbilitySystemGlobals.h"

UOrionCombatAssetManager& UOrionCombatAssetManager::Get()
{
	check(GEngine);

	UOrionCombatAssetManager* OrionCombatAssetManager = Cast<UOrionCombatAssetManager>(GEngine->AssetManager);
	return *OrionCombatAssetManager;
}

void UOrionCombatAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	// This is required to use Target Data!
	UAbilitySystemGlobals::Get().InitGlobalData();
}
