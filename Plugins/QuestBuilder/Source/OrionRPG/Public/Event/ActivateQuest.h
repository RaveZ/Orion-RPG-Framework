// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/OrionEvent.h"
#include "ActivateQuest.generated.h"

/**
 * Activate quest event node.
 * Desc
 */
UCLASS()
class ORIONRPG_API UActivateQuest : public UOrionEvent
{
	GENERATED_BODY()
public:
	UActivateQuest();

	virtual void BeginEvent_Implementation(AController* OwnerController, APawn* ControlledPawn) override;
	virtual FString GetNodeDisplayText_Implementation() const override;
};
