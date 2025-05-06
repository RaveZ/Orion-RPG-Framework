// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderEdNode_Objective.h"
#include "QuestBuilderNode.h"

FText UQuestBuilderEdNode_Objective::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const UQuestBuilderNode* MyNode = Cast<UQuestBuilderNode>(QuestSystemGraphNode);

	if (MyNode != NULL)
	{
		return MyNode->GetNodeTitle();
	}
	else if (!ClassData.GetClassName().IsEmpty())
	{
		FString StoredClassName = ClassData.GetClassName();
		StoredClassName.RemoveFromEnd(TEXT("_C"));

		return FText::Format(NSLOCTEXT("QuestGraph", "NodeClassError", "Class {0} not found, make sure it's saved!"), FText::FromString(StoredClassName));
	}

	return Super::GetNodeTitle(TitleType);
}
