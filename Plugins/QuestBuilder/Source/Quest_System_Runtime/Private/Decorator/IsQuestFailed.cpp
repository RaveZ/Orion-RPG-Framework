// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "Decorator/IsQuestFailed.h"
#include "QuestComponent.h"
#include "Quest.h"

UIsQuestFailed::UIsQuestFailed()
{
    bUseQuestID = true;
}

bool UIsQuestFailed::IsConditionMet_Implementation() const
{
    if (UQuest* FoundedQuest = GetQuestComponent()->QuestMap.FindRef(QuestID))
    {
        switch (FoundedQuest->QuestState)
        {
        case EQuestState::E_Fail:
            return true;
        default:
            return false;
        }
    }
    return false;
}

FString UIsQuestFailed::GetNodeDisplayText_Implementation() const
{
    return FString::Printf(TEXT("Is Quest Failed :  %s"), *QuestID.ToString());
}
