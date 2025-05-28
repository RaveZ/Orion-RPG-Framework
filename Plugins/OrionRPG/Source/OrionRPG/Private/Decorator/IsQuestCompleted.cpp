// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Decorator/IsQuestCompleted.h"
#include "QuestComponent.h"
#include "Quest.h"

UIsQuestCompleted::UIsQuestCompleted()
{
    bUseQuestTag = true;
}

bool UIsQuestCompleted::PerformConditionCheck_Implementation(AController* OwnerController, APawn* ControlledPawn) const
{
    FName QuestID = QuestTag.GetTagName();
    if (UQuest* FoundedQuest = GetQuestComponent()->QuestMap.FindRef(QuestID))
    {
        return GetQuestComponent()->IsQuestAtState(QuestTag, EQuestState::E_Complete);
    }
    return false;
}

FString UIsQuestCompleted::GetNodeDisplayText_Implementation() const
{
    return FString::Printf(TEXT("Is Quest Completed :  %s"), *GetShortTag(QuestTag));
}
