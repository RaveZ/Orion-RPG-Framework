// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderEdNode_PlayerOption.h"
#include "DialogBuilderSetting.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderNode_PlayerOption.h"

UDialogBuilderEdNode_PlayerOption::UDialogBuilderEdNode_PlayerOption()
{
}

FText UDialogBuilderEdNode_PlayerOption::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const UDialogBuilderNode* MyNode = Cast<UDialogBuilderNode>(NodeInstance);
	if (MyNode != NULL)
	{
		return MyNode->GetNodeTitle();
	}

	return Super::GetNodeTitle(TitleType);
}

FText UDialogBuilderEdNode_PlayerOption::GetTooltipText() const
{
	const UDialogBuilderNode_PlayerOption* PlayerOptionNode = Cast<UDialogBuilderNode_PlayerOption>(NodeInstance);
	if (PlayerOptionNode)
	{
		return FText::Format(FText::FromString(TEXT("Player Option Node\n{0}")),
			NSLOCTEXT("DialogBuilderEditor", "PlayerOptionNodeScopeTooltip", "This Node signifies the conclusion point of a dialog, determining its outcome, either Completed or Failed.\nThis node contain series of events, and decorators.\n**Right click the node to start adding a subnodes."));
	}

	return Super::GetTooltipText();
}

FLinearColor UDialogBuilderEdNode_PlayerOption::GetBackgroundColor() const
{
	return GetDefault<UDialogBuilderSetting>()->PlayerOptionNodeColor;
}

void UDialogBuilderEdNode_PlayerOption::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	AddContextMenuActionsEvents(Menu, "DialogBuilderEdNode", Context);
	AddContextMenuActionsDecorators(Menu, "DialogBuilderEdNode", Context);
}
