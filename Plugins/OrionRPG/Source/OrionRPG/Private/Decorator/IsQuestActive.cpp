// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Decorator/IsQuestActive.h"
#include "QuestComponent.h"
#include "Quest.h"

UIsQuestActive::UIsQuestActive()
{
    bUseQuestTag = true;
}

bool UIsQuestActive::PerformConditionCheck_Implementation(AController* OwnerController, APawn* ControlledPawn) const
{
    FName QuestID = QuestTag.GetTagName();
    if (UQuest* FoundedQuest = GetQuestComponent()->QuestMap.FindRef(QuestID))
    {
        return GetQuestComponent()->IsQuestAtState(QuestTag, EQuestState::E_Active);
    }
    return false;
}

FString UIsQuestActive::GetNodeDisplayText_Implementation() const
{
    return FString::Printf(TEXT("Is Quest Active :  %s"), *GetShortTag(QuestTag));
}
