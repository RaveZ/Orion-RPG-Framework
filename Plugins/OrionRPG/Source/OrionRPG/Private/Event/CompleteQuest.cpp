// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Event/CompleteQuest.h"
#include "Quest.h"
#include "QuestComponent.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"

UCompleteQuest::UCompleteQuest()
{
	bUseQuestTag = true;
}

void UCompleteQuest::BeginEvent(AController* OwnerController, APawn* ControlledPawn)
{
	if (QuestTag.IsValid())
	{
		GetQuestComponent()->CompleteQuestFromTag(QuestTag);
	}
	EndEvent();
	
}

FString UCompleteQuest::GetNodeDisplayText_Implementation() const
{
	return FString::Printf(TEXT("Fail Quest :  %s"), *GetShortTag(QuestTag));
}
