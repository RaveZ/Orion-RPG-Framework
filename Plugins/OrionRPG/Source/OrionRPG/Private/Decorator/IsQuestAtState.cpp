// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Decorator/IsQuestAtState.h"
#include "QuestComponent.h"
#include "QuestBuilderFunctionLibrary.h"
#include "Quest.h"

UIsQuestAtState::UIsQuestAtState()
{
    bUseQuestTag = true;
}

bool UIsQuestAtState::PerformConditionCheck_Implementation(AController* OwnerController, APawn* ControlledPawn) const
{
    FName QuestID = QuestTag.GetTagName();
    if (GetWorld())
    {
        if (UQuestComponent* QuestComp = UQuestBuilderFunctionLibrary::GetQuestComponent(GetWorld()))
        {
            if (UQuest* FoundedQuest = QuestComp->QuestMap.FindRef(QuestID))
            {
                return QuestComp->IsQuestAtState(QuestTag, QuestState);
            }
        }
    }
    return false;
}

FString UIsQuestAtState::GetNodeDisplayText_Implementation() const
{
    return FString::Printf(TEXT("Is Quest Active :  %s"), *GetShortTag(QuestTag));
}
