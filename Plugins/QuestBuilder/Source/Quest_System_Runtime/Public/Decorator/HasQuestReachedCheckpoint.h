// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Decorator/OrionDecorator.h"
#include "HasQuestReachedCheckpoint.generated.h"

/**
 * Is objective completed decorator node.
 * Desc
 */
UCLASS()
class QUEST_SYSTEM_RUNTIME_API UHasQuestReachedCheckpoint : public UOrionDecorator
{
	GENERATED_BODY()
public:
	UHasQuestReachedCheckpoint();


	virtual bool PerformConditionCheck_Implementation(AController* OwnerController, APawn* ControlledPawn) const override;
	virtual FString GetNodeDisplayText_Implementation() const override;

	virtual TArray<FName> GetAllNodeID() override;
};
