// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Decorator/IsQuestActive.h"
#include "QuestComponent.h"
#include "Quest.h"

UIsQuestActive::UIsQuestActive()
{
    bUseQuestID = true;
}

bool UIsQuestActive::PerformConditionCheck_Implementation(AController* OwnerController, APawn* ControlledPawn) const
{
    if (UQuest* FoundedQuest = GetQuestComponent()->QuestMap.FindRef(QuestID))
    {
        switch (FoundedQuest->QuestState)
        {
            case EQuestState::E_Active :
                return true;
            default:
                return false;
        }
    }
    return false;
}

FString UIsQuestActive::GetNodeDisplayText_Implementation() const
{
    return FString::Printf(TEXT("Is Quest Active :  %s"), *QuestID.ToString());
}
