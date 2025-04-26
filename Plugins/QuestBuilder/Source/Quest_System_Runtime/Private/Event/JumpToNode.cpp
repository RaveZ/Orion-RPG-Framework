// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "Event/JumpToNode.h"
#include "QuestSystemGraphNode.h"
#include "QuestSystemGraphNode_Root.h"
#include "QuestSystemGraph.h"
#include "QuestSystem.h"
#include "QuestComponent.h"

UJumpToNode::UJumpToNode()
{
	bUseNodeID = true;
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
