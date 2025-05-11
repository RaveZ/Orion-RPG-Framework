// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderEdNode_Objective.h"
#include "DialogBuilderSetting.h"
#include "QuestBuilderNode.h"

FText UDialogBuilderEdNode_Objective::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const UQuestBuilderNode* QuestNode = Cast<UQuestBuilderNode>(NodeInstance);

	if (QuestNode != NULL)
	{
		return QuestNode->GetNodeTitle();
	}
	else if (!ClassData.GetClassName().IsEmpty())
	{
		FString StoredClassName = ClassData.GetClassName();
		StoredClassName.RemoveFromEnd(TEXT("_C"));

		return FText::Format(NSLOCTEXT("QuestGraph", "NodeClassError", "Class {0} not found, make sure it's saved!"), FText::FromString(StoredClassName));
	}

	return Super::GetNodeTitle(TitleType);
}

FLinearColor UDialogBuilderEdNode_Objective::GetBackgroundColor() const
{
	return GetDefault<UDialogBuilderSetting>()->ObjectiveNodeColor;
}
