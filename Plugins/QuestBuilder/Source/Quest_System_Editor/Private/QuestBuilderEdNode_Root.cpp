// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderEdNode_Root.h"

UQuestBuilderEdNode_Root::UQuestBuilderEdNode_Root()
{
	bIsReadOnly = true;
	bCanRenameNode = false;
}

void UQuestBuilderEdNode_Root::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, "MultipleNode", TEXT("In"));
}

FText UQuestBuilderEdNode_Root::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("BehaviorTreeEditor", "Root", "ROOT");
	
}
