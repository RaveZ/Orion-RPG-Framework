// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderEdNode_Objective.h"
#include "DialogBuilderNode_Objective.h"
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

FText UDialogBuilderEdNode_Objective::GetTooltipText() const
{
	const UDialogBuilderNode_Objective* ObjectiveNode = Cast<UDialogBuilderNode_Objective>(NodeInstance);
	if (ObjectiveNode)
	{
		return FText::Format(FText::FromString(TEXT("Objective Node\n{0}")),
			NSLOCTEXT("DialogBuilderEditor", "ObjectiveNodeScopeTooltip", "This Node serves as the central unit within your dialog graph.\nEncapsulating a series of tasks, events, and decorators that define the player's objectives.\n**Right click the node to start adding a subnodes."));
	}

	return Super::GetTooltipText();
}

void UDialogBuilderEdNode_Objective::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	AddContextMenuActionsEvents(Menu, "DialogBuilderEdNode", Context);
	AddContextMenuActionsDecorators(Menu, "DialogBuilderEdNode", Context);
}
