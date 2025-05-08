// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "Condition/IsQuestCompleted.h"
#include "QuestComponent.h"
#include "Quest.h"

UIsQuestCompleted::UIsQuestCompleted()
{
    bUseQuestID = true;
}

bool UIsQuestCompleted::IsConditionMet_Implementation() const
{
    if (UQuest* FoundedQuest = GetQuestComponent()->QuestMap.FindRef(QuestID))
    {
        switch (FoundedQuest->QuestState)
        {
        case EQuestState::E_Complete:
            return true;
        default:
            return false;
        }
    }
    return false;
}

FString UIsQuestCompleted::GetNodeDisplayText_Implementation() const
{
    return FString::Printf(TEXT("Is Quest Completed :  %s"), *QuestID.ToString());
}
