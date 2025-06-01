// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Decorator/IsQuestAtState.h"
#include "QuestComponent.h"
#include "Quest.h"

UIsQuestAtState::UIsQuestAtState()
{
    bUseQuestTag = true;
}

bool UIsQuestAtState::PerformConditionCheck_Implementation(AController* OwnerController, APawn* ControlledPawn) const
{
    FName QuestID = QuestTag.GetTagName();
    if (UQuest* FoundedQuest = GetQuestComponent()->QuestMap.FindRef(QuestID))
    {
        return GetQuestComponent()->IsQuestAtState(QuestTag, QuestState);
    }
    return false;
}

FString UIsQuestAtState::GetNodeDisplayText_Implementation() const
{
    return FString::Printf(TEXT("Is Quest Active :  %s"), *GetShortTag(QuestTag));
}
