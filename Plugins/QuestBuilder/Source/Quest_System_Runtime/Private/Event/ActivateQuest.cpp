// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Event/ActivateQuest.h"
#include "Quest.h"
#include "QuestComponent.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"

UActivateQuest::UActivateQuest()
{
	bUseQuestID = true;
}

void UActivateQuest::BeginEvent_Implementation(AController* OwnerController, APawn* ControlledPawn)
{
	if (!QuestID.IsNone())
	{
		GetQuestComponent()->ActivateQuest(QuestID);
	}
	
}

FString UActivateQuest::GetNodeDisplayText_Implementation() const
{
	return FString::Printf(TEXT("Activate Quest :  %s"), *QuestID.ToString());
}
