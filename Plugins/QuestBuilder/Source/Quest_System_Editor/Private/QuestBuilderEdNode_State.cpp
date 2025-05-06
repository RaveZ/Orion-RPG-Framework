// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderEdNode_State.h"
#include "QuestBuilderNode.h"

UQuestBuilderEdNode_State::UQuestBuilderEdNode_State()
{
}

void UQuestBuilderEdNode_State::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, "MultipleNode", TEXT("In"));
}

FText UQuestBuilderEdNode_State::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const UQuestBuilderNode* MyNode = Cast<UQuestBuilderNode>(QuestSystemGraphNode);
	if (MyNode != NULL)
	{
		return MyNode->GetNodeTitle();
	}

	return Super::GetNodeTitle(TitleType);
}
