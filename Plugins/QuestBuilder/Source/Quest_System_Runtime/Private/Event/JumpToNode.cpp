// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Event/JumpToNode.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderNode_Root.h"
#include "QuestBuilderGraph.h"
#include "Quest.h"
#include "QuestComponent.h"

UJumpToNode::UJumpToNode()
{
	bUseNodeID = true;
}

void UJumpToNode::BeginEvent_Implementation(AController* OwnerController, APawn* ControlledPawn)
{
	if (UQuestBuilderNode* FoundedNode = GetOwningQuest()->NodeMap.FindRef(NodeID))
	{
		GetQuestComponent()->GoToNextNode(GetOwningQuest(), FoundedNode);
	}
}

FString UJumpToNode::GetNodeDisplayText_Implementation() const
{
	return FString::Printf(TEXT("Jump To Node :  %s"), *NodeID.ToString());
}
