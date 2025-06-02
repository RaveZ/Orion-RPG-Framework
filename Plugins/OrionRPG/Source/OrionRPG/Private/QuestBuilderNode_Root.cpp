// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderNode_Root.h"
#include "QuestComponent.h"
#include "QuestBuilderNode.h"


#if WITH_EDITOR
void UQuestBuilderNode_Root::BeginNode()
{
	Super::BeginNode();
	Quest->UpdateQuest();
	Deinitialize();
}

FText UQuestBuilderNode_Root::GetNodeDescription() const
{
	return FText::FromString(TEXT("Root Node"));
}
#endif