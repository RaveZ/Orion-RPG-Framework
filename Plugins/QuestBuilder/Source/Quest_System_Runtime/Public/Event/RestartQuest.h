// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/OrionEvent.h"
#include "RestartQuest.generated.h"

/**
 * 
 */


UCLASS()
class QUEST_SYSTEM_RUNTIME_API URestartQuest : public UOrionEvent
{
	GENERATED_BODY()
public:
	URestartQuest();

	
	virtual void BeginEvent_Implementation() override;
	virtual FString GetNodeDisplayText_Implementation() const override;

};
