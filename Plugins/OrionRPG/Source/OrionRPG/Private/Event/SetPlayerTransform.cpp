// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Event/SetPlayerTransform.h"
#include "Quest.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "QuestComponent.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"

USetPlayerTransform::USetPlayerTransform()
{
}

void USetPlayerTransform::BeginEvent(AController* OwnerController, APawn* ControlledPawn)
{
	if (bUsePlayerStart)
	{
		AGameModeBase* GameMode = UGameplayStatics::GetGameMode(GetWorld());
		if (GameMode)
		{
			AActor* PlayerStart =  GameMode->FindPlayerStart(OwnerController, PlayerStartTag);
			if (PlayerStart)
			{
				ControlledPawn->TeleportTo(PlayerStart->GetActorLocation(), PlayerStart->GetActorRotation());
			}
		}
	}
	else
	{
		ControlledPawn->TeleportTo(Transform.GetLocation(), Transform.Rotator());
	}

	EndEvent();
}

FString USetPlayerTransform::GetNodeDisplayText_Implementation() const
{
	if (bUsePlayerStart)
	{
		return FString::Printf(TEXT("Set Player Transform In Player Start:  %s"), *PlayerStartTag);
	}

	return FString::Printf(TEXT("Set Player Transform"));
	
}
