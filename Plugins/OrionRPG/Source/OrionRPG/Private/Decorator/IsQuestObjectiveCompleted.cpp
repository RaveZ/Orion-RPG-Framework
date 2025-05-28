// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Decorator/IsQuestObjectiveCompleted.h"
#include "QuestBuilderNode_Objective.h"
#include "QuestComponent.h"
#include "Quest.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "QuestBuilderGraph.h"

UIsQuestObjectiveCompleted::UIsQuestObjectiveCompleted()
{
    bUseQuestTag = true;
	bUseNodeTag = true;
}

bool UIsQuestObjectiveCompleted::PerformConditionCheck_Implementation(AController* OwnerController, APawn* ControlledPawn) const
{
    FName QuestID = QuestTag.GetTagName();
    FName NodeID = NodeTag.GetTagName();
    if (UQuest* FoundedQuest = GetQuestComponent()->QuestMap.FindRef(QuestID))
    {
        UQuestBuilderNode* FoundedNode = FoundedQuest->NodeMap.FindRef(NodeID);
        if (UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(FoundedNode))
        {
            return ObjectiveNode->IsAllTaskCompleted();
        }
    }
    return false;
}

FString UIsQuestObjectiveCompleted::GetNodeDisplayText_Implementation() const
{
    return FString::Printf(TEXT("In Quest: %s \nIn Objective: %s"), *GetShortTag(QuestTag), *GetShortTag(NodeTag));
}
