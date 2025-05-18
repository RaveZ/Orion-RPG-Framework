// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Event/JumpToNode.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderNode_Root.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderFunctionLibrary.h"
#include "Quest.h"
#include "QuestComponent.h"

UJumpToNode::UJumpToNode()
{
	bUseQuestID = true;
	bUseNodeID = true;
}

void UJumpToNode::BeginEvent_Implementation(AController* OwnerController, APawn* ControlledPawn)
{
	if (UQuest* FoundedQuest = UQuestBuilderFunctionLibrary::FindQuestFromNodeID(GetWorld(), NodeID))
	{
		UQuestBuilderNode* FoundedNode = FoundedQuest->NodeMap.FindRef(NodeID);
		if (FoundedNode)
		{
			GetQuestComponent()->GoToNextNode(FoundedQuest, FoundedNode);
		}
	}
}

FString UJumpToNode::GetNodeDisplayText_Implementation() const
{
	return FString::Printf(TEXT("Jump To Node :  %s"), *NodeID.ToString());
}
