// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "OrionAIController.h"

#include "../Character/OrionCharacterBase.h"

AOrionAIController::AOrionAIController()
{
}

void AOrionAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (Behaviour && InPawn)
	{
		RunBehaviorTree(Behaviour);
	}
}

FVector AOrionAIController::GetFocalPointOnActor(const AActor* Actor) const
{
	if (Actor == nullptr)
	{
		return FAISystem::InvalidLocation;
	}
	const APawn* FocusPawn = Cast<APawn>(Actor);
	if (FocusPawn)
	{
		// Focus on pawn's eye view point
		return FocusPawn->GetPawnViewLocation();
	}
	return Actor->GetActorLocation();
}
