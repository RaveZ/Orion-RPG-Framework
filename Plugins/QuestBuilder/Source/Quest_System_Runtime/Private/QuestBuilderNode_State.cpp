// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderNode_State.h"
#include "QuestComponent.h"
#include "Quest.h"
#include "Event/QuestEvent.h"


void UQuestBuilderNode_State::BeginState()
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
FText UQuestBuilderNode_State::GetNodeTitle() const
{
    return FText::FromName(ID).IsEmpty() ? FText::FromString(UQuestBuilderNode::GetShortTypeName(this)) : FText::FromName(ID);
}
void UQuestBuilderNode_State::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}
FText UQuestBuilderNode_State::GetNodeDescription() const
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