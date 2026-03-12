// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LevelSequence.h"
#include "DialogSequence.generated.h"

UCLASS(BlueprintType)
class ORIONRPG_API UDialogSequence : public ULevelSequence
{
	GENERATED_BODY()

public:
	UDialogSequence(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};