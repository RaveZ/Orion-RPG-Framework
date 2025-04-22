// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "QuestSystemGraphNode_State.h"
#include "QuestComponent.h"
#include "QuestSystem.h"
#include "Event/QuestEvent.h"


void UQuestSystemGraphNode_State::BeginState()
{
    QuestSystem->QuestState = QuestState;

    switch (QuestState)
    {
    case EQuestState::E_Complete:
        GetQuestComponent()->OnQuestCompleted.Broadcast(QuestSystem);
        break;
    case EQuestState::E_Fail:
        GetQuestComponent()->OnQuestFailed.Broadcast(QuestSystem);
        break;
    }

    //End of a Node, Launch Event with the End and Both Type
    for (UQuestEvent* Event : QuestSystem->CurrentNode->Events)
    {
        if (Event)
        {
            if (Event->EventRunType == EEventRunType::E_Both || Event->EventRunType == EEventRunType::E_End)
            {
                Event->BeginEvent();
            }
        }
    }

    GetQuestComponent()->UpdateQuest();

    
}

#if WITH_EDITOR
FText UQuestSystemGraphNode_State::GetNodeTitle() const
{
    return FText::FromName(ID).IsEmpty() ? FText::FromString(UQuestSystemGraphNode::GetShortTypeName(this)) : FText::FromName(ID);
}
void UQuestSystemGraphNode_State::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}
FText UQuestSystemGraphNode_State::GetNodeDescription() const
{
    switch (QuestState)
    {
    case EQuestState::E_Complete:
        return FText::FromString(TEXT("Complete Quest"));
    case EQuestState::E_Fail:
        return FText::FromString(TEXT("Fail Quest"));
    default:
        return FText::FromString(TEXT("Default State"));
    }
}


FLinearColor UQuestSystemGraphNode_State::GetBackgroundColor() const
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