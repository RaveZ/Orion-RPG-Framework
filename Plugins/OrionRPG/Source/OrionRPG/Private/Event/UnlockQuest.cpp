// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Event/UnlockQuest.h"
#include "Quest.h"
#include "QuestComponent.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"

UUnlockQuest::UUnlockQuest()
{
	bUseQuestTag = true;
}

void UUnlockQuest::BeginEvent(AController* OwnerController, APawn* ControlledPawn)
{
	if (QuestTag.IsValid())
	{
		GetQuestComponent()->UnlockQuestFromTag(QuestTag);
	}
	EndEvent();
	
}

FString UUnlockQuest::GetNodeDisplayText_Implementation() const
{
	return FString::Printf(TEXT("Unlock Quest :  %s"), *GetShortTag(QuestTag));
}
