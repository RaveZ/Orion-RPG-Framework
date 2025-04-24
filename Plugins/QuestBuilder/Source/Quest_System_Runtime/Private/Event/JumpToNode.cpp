// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "Event/JumpToNode.h"
#include "QuestSystemGraphNode.h"
#include "QuestSystemGraphNode_Root.h"
#include "QuestSystemGraph.h"
#include "QuestSystem.h"
#include "QuestComponent.h"

UJumpToNode::UJumpToNode()
{
}

TArray<FName> UJumpToNode::GetNextNodeID()
{
	TArray<FName> NodeIDs;
	if(GetOwningQuestSystem())
	{
		for (UQuestSystemGraphNode* Node : GetOwningQuestSystem()->AllNodes)
		{
			if (Node)
			{
				if (UQuestSystemGraphNode_Root* RootNode = Cast<UQuestSystemGraphNode_Root>(Node)) continue;
				NodeIDs.Emplace(Node->ID);
			}
		}
	}
	return NodeIDs;
}

void UJumpToNode::BeginEvent_Implementation()
{
	if (UQuestSystemGraphNode* FoundedNode = GetOwningQuestSystem()->NodeMap.FindRef(NodeID))
	{
		GetQuestComponent()->GoToNextNode(GetOwningQuestSystem(), FoundedNode);
	}
}

FString UJumpToNode::GetNodeDisplayText_Implementation() const
{
	return FString::Printf(TEXT("Jump To Node :  %s"), *NodeID.ToString());
}
