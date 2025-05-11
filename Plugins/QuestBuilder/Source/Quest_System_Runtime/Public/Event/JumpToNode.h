// Copyright 2024 Ivan Chandra. All Rights Reserved.

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

	virtual void BeginEvent_Implementation() override;
	virtual FString GetNodeDisplayText_Implementation() const override;
};
