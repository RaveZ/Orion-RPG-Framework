// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderEdNode_State.h"
#include "DialogBuilderSetting.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderNode_State.h"

UDialogBuilderEdNode_State::UDialogBuilderEdNode_State()
{
}

void UDialogBuilderEdNode_State::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, "MultipleNode", TEXT("In"));
}

FText UDialogBuilderEdNode_State::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const UQuestBuilderNode* MyNode = Cast<UQuestBuilderNode>(NodeInstance);
	if (MyNode != NULL)
	{
		return MyNode->GetNodeTitle();
	}

	return Super::GetNodeTitle(TitleType);
}

FLinearColor UDialogBuilderEdNode_State::GetBackgroundColor() const
{
	FLinearColor Default(0.05f, 0.05f, 0.05f);
	if (UQuestBuilderNode_State* StateNode = Cast<UQuestBuilderNode_State>(NodeInstance))
	{

		switch (StateNode->QuestState)
		{
		case EQuestState::E_Complete:
			return GetDefault<UDialogBuilderSetting>()->SuccessNodeColor;
		case EQuestState::E_Fail:
			return GetDefault<UDialogBuilderSetting>()->FailedNodeColor;
		}
	}

	return GetDefault<UDialogBuilderSetting>()->RootNodeColor;
}
