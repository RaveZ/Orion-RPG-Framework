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
}

TArray<FName> URestartQuest::GetAllNodeIDFromQuest()
{
	TArray<FName> NodeIDs;
	if(GetOwningQuestGraph())
	{
		for(UQuestSystem* Quest : GetOwningQuestGraph()->QuestSystems)
		{
			if (Quest)
			{
				if (Quest->QuestID != QuestID) continue;
				for (UQuestSystemGraphNode* Node : Quest->AllNodes)
				{
					if (Node)
					{
						if (UQuestSystemGraphNode_Root* RootNode = Cast<UQuestSystemGraphNode_Root>(Node)) continue;
						NodeIDs.Emplace(Node->ID);
					}
				}
			}
			
		}
		
	}
	return NodeIDs;
}

void URestartQuest::BeginEvent_Implementation()
{
	if (!QuestID.IsNone())
	{
		GetQuestComponent()->RestartQuest(QuestID, NodeID, bStartFromNodeID);
	}
}

FString URestartQuest::GetNodeDisplayText_Implementation() const
{
    if(bStartFromNodeID)
    {  
		return FString::Printf(TEXT("Restart Quest :  %s From %s"), *QuestID.ToString(), *NodeID.ToString());
	}
	else
	{
		return FString::Printf(TEXT("Restart Quest :  %s"), *QuestID.ToString());
	}

}

