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
        break;
    case EDialogState::E_Fail:
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
    FString StateDesc;

    switch (DialogState)
    {
    case EDialogState::E_Complete:
        StateDesc = "Complete Dialog";
        break;
    case EDialogState::E_Fail:
        StateDesc = "Fail Dialog";
        break;
    default:
        StateDesc = "Default State";
        break;
    }

	StateDesc += "\n" + Description.ToString();

	return FText::FromString(StateDesc);
}


FLinearColor UDialogBuilderNode_State::GetBackgroundColor() const
{
    FLinearColor FailDialogColor(.34f, .024f, .024f);
    FLinearColor CompleteDialogColor(0.20f, 0.32f, 0.15f);
    FLinearColor Default(0.05f, 0.05f, 0.05f);

    switch (DialogState)
    {
        case EDialogState::E_Complete:
            return CompleteDialogColor;
        case EDialogState::E_Fail:
            return FailDialogColor;
        default:
            return Default;
    }
}
#endif