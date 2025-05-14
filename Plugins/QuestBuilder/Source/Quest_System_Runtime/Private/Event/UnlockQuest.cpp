// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "Event/UnlockQuest.h"
#include "Quest.h"
#include "QuestComponent.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"

UUnlockQuest::UUnlockQuest()
{
	bUseQuestID = true;
}

void UUnlockQuest::BeginEvent_Implementation(AController* OwnerController, APawn* ControlledPawn)
{
	if (!QuestID.IsNone())
	{
		GetQuestComponent()->UnlockQuest(QuestID);
	}
	
}

FString UUnlockQuest::GetNodeDisplayText_Implementation() const
{
	return FString::Printf(TEXT("Unlock Quest :  %s"), *QuestID.ToString());
}
