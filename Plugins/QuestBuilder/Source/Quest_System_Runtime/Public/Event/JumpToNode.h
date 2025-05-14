// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/OrionEvent.h"
#include "JumpToNode.generated.h"

/**
 * 
 */
UCLASS()
class QUEST_SYSTEM_RUNTIME_API UJumpToNode : public UOrionEvent
{
	GENERATED_BODY()
public:
	UJumpToNode();

	virtual void BeginEvent_Implementation(AController* OwnerController, APawn* ControlledPawn) override;
	virtual FString GetNodeDisplayText_Implementation() const override;
};
