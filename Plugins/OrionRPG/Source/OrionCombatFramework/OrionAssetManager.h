// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "OrionAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class ORIONCOMBATFRAMEWORK_API UOrionAssetManager : public UAssetManager
{
	GENERATED_BODY()
	static UOrionAssetManager& Get();

protected:

	virtual void StartInitialLoading() override;
};
