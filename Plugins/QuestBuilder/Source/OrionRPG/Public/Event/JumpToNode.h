// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/OrionEvent.h"
#include "JumpToNode.generated.h"

/**
 * Jump to node event node.
 * Desc
 */
UCLASS()
class ORIONRPG_API UJumpToNode : public UOrionEvent
{
	GENERATED_BODY()
public:
	UJumpToNode();

	virtual void BeginEvent_Implementation(AController* OwnerController, APawn* ControlledPawn) override;
	virtual FString GetNodeDisplayText_Implementation() const override;
};
