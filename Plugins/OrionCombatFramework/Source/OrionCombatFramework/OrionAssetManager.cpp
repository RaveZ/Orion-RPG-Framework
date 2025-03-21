// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionAssetManager.h"
#include "AbilitySystemGlobals.h"

UOrionAssetManager& UOrionAssetManager::Get()
{
	check(GEngine);

	UOrionAssetManager* OrionAssetManager = Cast<UOrionAssetManager>(GEngine->AssetManager);
	return *OrionAssetManager;
}

void UOrionAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	// This is required to use Target Data!
	UAbilitySystemGlobals::Get().InitGlobalData();
}
