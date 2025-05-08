// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "Condition/IsObjectiveCompleted.h"
#include "QuestBuilderNode_Objective.h"
#include "QuestComponent.h"
#include "Quest.h"

UIsObjectiveCompleted::UIsObjectiveCompleted()
{
    bUseQuestID = true;
	bUseNodeID = true;
}

bool UIsObjectiveCompleted::IsConditionMet_Implementation() const
{

    for (const auto& It : GetQuestComponent()->QuestMap)
    {
        FName Key = It.Key;
        UQuest* QuestObj = It.Value;

        if (UQuestBuilderNode_Objective* ObjectiveNode =  Cast<UQuestBuilderNode_Objective>(QuestObj->NodeMap.FindRef(NodeID)))
        {
            return ObjectiveNode->IsAllTaskCompleted();
        }
    }
    return false;
}

FString UIsObjectiveCompleted::GetNodeDisplayText_Implementation() const
{
    return FString::Printf(TEXT("Is Objective Completed :  %s"), *NodeID.ToString());
}
