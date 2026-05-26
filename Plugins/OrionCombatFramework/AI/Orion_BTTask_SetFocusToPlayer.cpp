// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Orion_BTTask_SetFocusToPlayer.h"
#include "Engine/World.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "AIController.h"

UOrion_BTTask_SetFocusToPlayer::UOrion_BTTask_SetFocusToPlayer()
{
	NodeName = "Focus On Player";
}

EBTNodeResult::Type UOrion_BTTask_SetFocusToPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (Pawn)
	{
		OwnerComp.GetAIOwner()->SetFocus(Pawn);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

FString UOrion_BTTask_SetFocusToPlayer::GetStaticDescription() const
{
	return "Set Focus to player's pawn";
}
