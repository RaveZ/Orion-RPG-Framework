// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderEdNode_State.h"
#include "QuestSystemGraphNode.h"

UQuestBuilderEdNode_State::UQuestBuilderEdNode_State()
{
}

void UQuestBuilderEdNode_State::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, "MultipleNode", TEXT("In"));
}

FText UQuestBuilderEdNode_State::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const UQuestSystemGraphNode* MyNode = Cast<UQuestSystemGraphNode>(QuestSystemGraphNode);
	if (MyNode != NULL)
	{
		return MyNode->GetNodeTitle();
	}

	return Super::GetNodeTitle(TitleType);
}
