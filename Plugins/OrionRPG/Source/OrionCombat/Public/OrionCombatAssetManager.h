// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "OrionCombatAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class ORIONCOMBAT_API UOrionCombatAssetManager : public UAssetManager
{
	GENERATED_BODY()
	static UOrionCombatAssetManager& Get();

protected:

	virtual void StartInitialLoading() override;
};
