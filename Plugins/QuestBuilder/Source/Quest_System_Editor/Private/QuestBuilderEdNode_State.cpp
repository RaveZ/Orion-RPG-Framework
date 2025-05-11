// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderEdNode_State.h"
#include "QuestBuilderSetting.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderNode_State.h"

UQuestBuilderEdNode_State::UQuestBuilderEdNode_State()
{
}

void UQuestBuilderEdNode_State::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, "MultipleNode", TEXT("In"));
}

FText UQuestBuilderEdNode_State::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const UQuestBuilderNode* MyNode = Cast<UQuestBuilderNode>(NodeInstance);
	if (MyNode != NULL)
	{
		return MyNode->GetNodeTitle();
	}

	return Super::GetNodeTitle(TitleType);
}

FLinearColor UQuestBuilderEdNode_State::GetBackgroundColor() const
{
	FLinearColor Default(0.05f, 0.05f, 0.05f);
	if (UQuestBuilderNode_State* StateNode = Cast<UQuestBuilderNode_State>(NodeInstance))
	{

		switch (StateNode->QuestState)
		{
		case EQuestState::E_Complete:
			return GetDefault<UQuestBuilderSetting>()->SuccessNodeColor;
		case EQuestState::E_Fail:
			return GetDefault<UQuestBuilderSetting>()->FailedNodeColor;
		}
	}

	return GetDefault<UQuestBuilderSetting>()->RootNodeColor;
}
