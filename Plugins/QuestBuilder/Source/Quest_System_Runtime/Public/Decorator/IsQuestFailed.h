// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Decorator/OrionDecorator.h"
#include "IsQuestFailed.generated.h"

/**
 * 
 */
UCLASS()
class QUEST_SYSTEM_RUNTIME_API UIsQuestFailed : public UOrionDecorator
{
	GENERATED_BODY()

public:
	UIsQuestFailed();


	virtual bool PerformConditionCheck_Implementation(AController* OwnerController, APawn* ControlledPawn) const override;
	virtual FString GetNodeDisplayText_Implementation() const override;
};
