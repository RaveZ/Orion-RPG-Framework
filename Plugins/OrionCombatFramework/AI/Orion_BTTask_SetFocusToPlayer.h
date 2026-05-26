// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Orion_BTTask_SetFocusToPlayer.generated.h"

/** Set AIController's Focus to the Player's Pawn Actor. */
UCLASS(Category = Orion, meta = (DisplayName = "Set Focus to Player"))
class ORIONCOMBATFRAMEWORK_API UOrion_BTTask_SetFocusToPlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UOrion_BTTask_SetFocusToPlayer();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};
