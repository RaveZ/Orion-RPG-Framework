// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderNode_Objective.h"
#include "Task/QuestTask.h"
#include "Quest.h"
#include "QuestComponent.h"


#define LOCTEXT_NAMESPACE "QuestSystemGraphNode_TaskManager"

void UQuestBuilderNode_Objective::Initialize()
{
	BeginAllTask();
}

void UQuestBuilderNode_Objective::BeginAllTask()
{
    for (auto& Task : Tasks)
    {
        if (Task)
        {
            Task->BeginTask();
        }

    }
}

bool UQuestBuilderNode_Objective::IsAllTaskCompleted()
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
FText UQuestBuilderNode_Objective::GetNodeTitle() const
{
    return FText::FromName(ID).IsEmpty() ? FText::FromString(UQuestBuilderNode::GetShortTypeName(this)) : FText::FromName(ID);
}

void UQuestBuilderNode_Objective::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}

FText UQuestBuilderNode_Objective::GetNodeDescription() const
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