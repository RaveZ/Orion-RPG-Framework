// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "Event/FailQuest.h"
#include "Quest.h"
#include "QuestComponent.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"

UFailQuest::UFailQuest()
{
	bUseQuestID = true;
}

void UFailQuest::BeginEvent_Implementation()
{
	if (!QuestID.IsNone())
	{
		GetQuestComponent()->FailQuest(QuestID);
	}
	
}

FString UFailQuest::GetNodeDisplayText_Implementation() const
{
	return FString::Printf(TEXT("Fail Quest :  %s"), *QuestID.ToString());
}
