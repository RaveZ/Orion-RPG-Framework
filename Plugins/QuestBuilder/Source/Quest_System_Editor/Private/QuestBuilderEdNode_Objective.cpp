// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderEdNode_Objective.h"
#include "QuestBuilderNode_Objective.h"
#include "QuestBuilderSetting.h"
#include "QuestBuilderNode.h"

FText UQuestBuilderEdNode_Objective::GetNodeTitle(ENodeTitleType::Type TitleType) const
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

FLinearColor UQuestBuilderEdNode_Objective::GetBackgroundColor() const
{
	return GetDefault<UQuestBuilderSetting>()->ObjectiveNodeColor;
}

FText UQuestBuilderEdNode_Objective::GetTooltipText() const
{
	const UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(NodeInstance);
	if (ObjectiveNode)
	{
		return FText::Format(FText::FromString(TEXT("Objective Node\n{0}")),
			NSLOCTEXT("QuestBuilderEditor", "ObjectiveNodeScopeTooltip", "This Node serves as the central unit within your quest graph.\nEncapsulating a series of tasks, events, and decorators that define the player's objectives.\nRight click the node to start adding a subnodes."));
	}

	return Super::GetTooltipText();
}

void UQuestBuilderEdNode_Objective::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	AddContextMenuActionsTasks(Menu, "QuestBuilderEdNode", Context);
	AddContextMenuActionsEvents(Menu, "QuestBuilderEdNode", Context);
	AddContextMenuActionsDecorators(Menu, "QuestBuilderEdNode", Context);
}
