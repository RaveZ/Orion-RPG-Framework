// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Decorator/IsQuestFailed.h"
#include "QuestComponent.h"
#include "QuestBuilderFunctionLibrary.h"
#include "Quest.h"

UIsQuestFailed::UIsQuestFailed()
{
    bUseQuestTag = true;
}

bool UIsQuestFailed::PerformConditionCheck_Implementation(AController* OwnerController, APawn* ControlledPawn) const
{
    FName QuestID = QuestTag.GetTagName();
    if (GetWorld())
    {
        if (UQuestComponent* QuestComp = UQuestBuilderFunctionLibrary::GetQuestComponent(GetWorld()))
        {
            if (UQuest* FoundedQuest = QuestComp->QuestMap.FindRef(QuestID))
            {
                return QuestComp->IsQuestAtState(QuestTag, EQuestState::E_Fail);
            }
        }
    }
    
    return false;
}

FString UIsQuestFailed::GetNodeDisplayText_Implementation() const
{
    return FString::Printf(TEXT("Is Quest Failed :  %s"), *GetShortTag(QuestTag));
}
