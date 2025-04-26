// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "Event/RestartQuest.h"
#include "QuestSystem.h"
#include "QuestComponent.h"
#include "QuestSystemGraph.h"
#include "QuestSystemGraphNode.h"
#include "QuestSystemGraphNode_Objective.h"
#include "QuestSystemGraphNode_Root.h"

URestartQuest::URestartQuest()
{
	bUseQuestID = true;
	bUseNodeID = true;
}

void URestartQuest::BeginEvent_Implementation()
{
	if (!QuestID.IsNone())
	{
		GetQuestComponent()->RestartQuest(QuestID, NodeID, bUseNodeID);
	}
}

FString URestartQuest::GetNodeDisplayText_Implementation() const
{
    if(!(NodeID == NAME_None))
    {  
		return FString::Printf(TEXT("Restart Quest :  %s From %s"), *QuestID.ToString(), *NodeID.ToString());
	}
	else
	{
		return FString::Printf(TEXT("Restart Quest :  %s"), *QuestID.ToString());
	}

}

