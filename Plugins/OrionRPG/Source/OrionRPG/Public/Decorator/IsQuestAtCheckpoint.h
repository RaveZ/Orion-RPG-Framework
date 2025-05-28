// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Decorator/OrionDecorator.h"
#include "IsQuestAtCheckpoint.generated.h"

/**
 * Is objective completed decorator node.
 * Desc
 */
UCLASS()
class ORIONRPG_API UIsQuestAtCheckpoint : public UOrionDecorator
{
	GENERATED_BODY()
public:
	UIsQuestAtCheckpoint();


	virtual bool PerformConditionCheck_Implementation(AController* OwnerController, APawn* ControlledPawn) const override;
	virtual FString GetNodeDisplayText_Implementation() const override;
};
