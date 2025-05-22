// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderNode_Root.h"
#include "QuestBuilderNode.h"


#if WITH_EDITOR
FText UQuestBuilderNode_Root::GetNodeDescription() const
{
	return FText::FromString(TEXT("Root Node"));
}
#endif