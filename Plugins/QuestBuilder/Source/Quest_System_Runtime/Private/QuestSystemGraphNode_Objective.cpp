// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "QuestSystemGraphNode_Objective.h"
#include "Task/QuestTask.h"
#include "QuestSystem.h"
#include "QuestComponent.h"


#define LOCTEXT_NAMESPACE "QuestSystemGraphNode_TaskManager"

bool UQuestSystemGraphNode_Objective::IsAllTaskCompleted()
{
    int progress = 0;
    int completeSum = 0;
    for (UQuestTask* Task : Tasks)
    {
        if (Task && !Task->bOptional)
        {
            completeSum++;
        }
        if (Task && !Task->bOptional && Task->IsTaskCompleted())
        {
            progress++;
            
        }
    }

    if (progress >= completeSum)
    {
        return true;
    }
    else
    {
        return false;
    }
    
}

#if WITH_EDITOR
FText UQuestSystemGraphNode_Objective::GetNodeTitle() const
{
    return FText::FromName(ID).IsEmpty() ? FText::FromString(UQuestSystemGraphNode::GetShortTypeName(this)) : FText::FromName(ID);
}

void UQuestSystemGraphNode_Objective::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}

FText UQuestSystemGraphNode_Objective::GetNodeDescription() const
{
    FString TaskDesc;

    for (const UQuestTask* Task : Tasks)
    {
        if (Task)
        {
            if(!TaskDesc.IsEmpty()) TaskDesc += TEXT("\n");
            TaskDesc += Task->GetDescription().ToString();
        }
        
    }

    return FText::FromString(TaskDesc);
}
#endif

#undef LOCTEXT_NAMESPACE