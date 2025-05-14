// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderNode_State.h"
#include "DialogComponent.h"
#include "Dialog.h"
#include "Event/OrionEvent.h"


void UDialogBuilderNode_State::Initialize()
{
	BeginState();
}

void UDialogBuilderNode_State::BeginState()
{
    Dialog->DialogState = DialogState;

    switch (DialogState)
    {
    case EDialogState::E_Complete:
        GetDialogComponent()->OnDialogCompleted.Broadcast(Dialog);
        break;
    case EDialogState::E_Fail:
        GetDialogComponent()->OnDialogFailed.Broadcast(Dialog);
        break;
    }

	BeginEvents(EEventRunType::E_End);

    GetDialogComponent()->UpdateDialog();

    
}

#if WITH_EDITOR
FText UDialogBuilderNode_State::GetNodeTitle() const
{
    return FText::FromName(ID).IsEmpty() ? FText::FromString(UDialogBuilderNode::GetShortTypeName(this)) : FText::FromName(ID);
}
void UDialogBuilderNode_State::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}
FText UDialogBuilderNode_State::GetNodeDescription() const
{
    switch (DialogState)
    {
    case EDialogState::E_Complete:
        return FText::FromString(TEXT("Complete Dialog"));
    case EDialogState::E_Fail:
        return FText::FromString(TEXT("Fail Dialog"));
    default:
        return FText::FromString(TEXT("Default State"));
    }
}


FLinearColor UDialogBuilderNode_State::GetBackgroundColor() const
{
    FLinearColor FailQuestColor(.34f, .024f, .024f);
    FLinearColor CompleteDialogColor(0.20f, 0.32f, 0.15f);
    FLinearColor Default(0.05f, 0.05f, 0.05f);

    switch (DialogState)
    {
        case EDialogState::E_Complete:
            return CompleteDialogColor;
        case EDialogState::E_Fail:
            return FailQuestColor;
        default:
            return Default;
    }
}
#endif