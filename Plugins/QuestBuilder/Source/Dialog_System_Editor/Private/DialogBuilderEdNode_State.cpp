// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderEdNode_State.h"
#include "DialogBuilderSetting.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderNode_State.h"

UDialogBuilderEdNode_State::UDialogBuilderEdNode_State()
{
}

void UDialogBuilderEdNode_State::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, "MultipleNode", TEXT("In"));
}

FText UDialogBuilderEdNode_State::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const UDialogBuilderNode* MyNode = Cast<UDialogBuilderNode>(NodeInstance);
	if (MyNode != NULL)
	{
		return MyNode->GetNodeTitle();
	}

	return Super::GetNodeTitle(TitleType);
}

FLinearColor UDialogBuilderEdNode_State::GetBackgroundColor() const
{
	FLinearColor Default(0.05f, 0.05f, 0.05f);
	if (UDialogBuilderNode_State* StateNode = Cast<UDialogBuilderNode_State>(NodeInstance))
	{

		switch (StateNode->DialogState)
		{
		case EDialogState::E_Complete:
			return GetDefault<UDialogBuilderSetting>()->SuccessNodeColor;
		case EDialogState::E_Fail:
			return GetDefault<UDialogBuilderSetting>()->FailedNodeColor;
		}
	}

	return GetDefault<UDialogBuilderSetting>()->RootNodeColor;
}
