// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderEdNode_Objective.h"
#include "DialogBuilderSetting.h"
#include "DialogBuilderNode.h"

FText UDialogBuilderEdNode_Objective::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const UDialogBuilderNode* DialogNode = Cast<UDialogBuilderNode>(NodeInstance);

	if (DialogNode != NULL)
	{
		return DialogNode->GetNodeTitle();
	}
	else if (!ClassData.GetClassName().IsEmpty())
	{
		FString StoredClassName = ClassData.GetClassName();
		StoredClassName.RemoveFromEnd(TEXT("_C"));

		return FText::Format(NSLOCTEXT("DialogGraph", "NodeClassError", "Class {0} not found, make sure it's saved!"), FText::FromString(StoredClassName));
	}

	return Super::GetNodeTitle(TitleType);
}

FLinearColor UDialogBuilderEdNode_Objective::GetBackgroundColor() const
{
	return GetDefault<UDialogBuilderSetting>()->ObjectiveNodeColor;
}
