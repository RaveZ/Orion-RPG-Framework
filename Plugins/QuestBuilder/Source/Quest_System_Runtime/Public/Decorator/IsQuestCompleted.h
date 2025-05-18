// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Decorator/OrionDecorator.h"
#include "IsQuestCompleted.generated.h"


class UQuest;
/**
 * Is quest completed decorator node.
 * Desc
 */
UCLASS()
class QUEST_SYSTEM_RUNTIME_API UIsQuestCompleted : public UOrionDecorator
{
	GENERATED_BODY()

public:
	UIsQuestCompleted();


	virtual bool PerformConditionCheck_Implementation(AController* OwnerController, APawn* ControlledPawn) const override;
	virtual FString GetNodeDisplayText_Implementation() const override;
};
