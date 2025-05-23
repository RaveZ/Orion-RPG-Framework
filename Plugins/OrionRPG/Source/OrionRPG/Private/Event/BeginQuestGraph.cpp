// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Event/BeginQuestGraph.h"
#include "Quest.h"
#include "QuestComponent.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"

UBeginQuestGraph::UBeginQuestGraph()
{
}

void UBeginQuestGraph::BeginEvent_Implementation(AController* OwnerController, APawn* ControlledPawn)
{
	if (GetQuestComponent())
	{
		GetQuestComponent()->BeginQuest(QuestAsset);
	}
	
	
}

FString UBeginQuestGraph::GetNodeDisplayText_Implementation() const
{
	return FString::Printf(TEXT("Begin Quest Graph :  %s"), *QuestAsset->GetName());
}
