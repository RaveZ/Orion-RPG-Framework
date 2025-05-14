// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Decorator/OrionDecorator.h"
#include "IsQuestActive.generated.h"

/**
 * 
 */
UCLASS()
class QUEST_SYSTEM_RUNTIME_API UIsQuestActive : public UOrionDecorator
{
	GENERATED_BODY()

public:
	UIsQuestActive();

	virtual bool PerformConditionCheck_Implementation(AController* OwnerController, APawn* ControlledPawn) const override;
	virtual FString GetNodeDisplayText_Implementation() const override;
};
