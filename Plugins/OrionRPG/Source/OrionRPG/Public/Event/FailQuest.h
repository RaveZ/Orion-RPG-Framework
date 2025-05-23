// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/OrionEvent.h"
#include "FailQuest.generated.h"

/**
 * Fail quest event node.
 * Desc
 */
UCLASS()
class ORIONRPG_API UFailQuest : public UOrionEvent
{
	GENERATED_BODY()
public:
	UFailQuest();

	virtual void BeginEvent_Implementation(AController* OwnerController, APawn* ControlledPawn) override;
	virtual FString GetNodeDisplayText_Implementation() const override;
};
