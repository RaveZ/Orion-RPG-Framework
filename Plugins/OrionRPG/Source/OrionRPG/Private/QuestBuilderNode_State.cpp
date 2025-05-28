// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderNode_State.h"
#include "QuestComponent.h"
#include "Quest.h"
#include "Event/OrionEvent.h"


void UQuestBuilderNode_State::Initialize(bool bLaunchEventOnLoad)
{
    Super::Initialize(bLaunchEventOnLoad);
	BeginState();
}

void UQuestBuilderNode_State::BeginState()
{
    Quest->QuestState = QuestState;

    switch (QuestState)
    {
    case EQuestState::E_Complete:
        GetQuestComponent()->OnQuestCompleted.Broadcast(Quest);
        break;
    case EQuestState::E_Fail:
        GetQuestComponent()->OnQuestFailed.Broadcast(Quest);
        break;
    }

    GetQuestComponent()->OnQuestObjectiveUpdated.Broadcast();
	Deinitialize();

    
}

#if WITH_EDITOR
FText UQuestBuilderNode_State::GetNodeTitle() const
{
    switch (QuestState)
    {
        case EQuestState::E_Complete:
            return FText::FromString("Complete Quest");
            break;
        case EQuestState::E_Fail:
            return FText::FromString("Fail Quest");
            break;
        default:
            return FText::FromString("Default");
            break;
    }
}
void UQuestBuilderNode_State::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}
FText UQuestBuilderNode_State::GetNodeDescription() const
{
    return Description;
}


FLinearColor UQuestBuilderNode_State::GetBackgroundColor() const
{
    FLinearColor FailQuestColor(.34f, .024f, .024f);
    FLinearColor CompleteQuestColor(0.20f, 0.32f, 0.15f);
    FLinearColor Default(0.05f, 0.05f, 0.05f);

    switch (QuestState)
    {
        case EQuestState::E_Complete:
            return CompleteQuestColor;
        case EQuestState::E_Fail:
            return FailQuestColor;
        default:
            return Default;
    }
}
#endif