// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderEdNode_State.h"
#include "DialogBuilderSetting.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderNode_State.h"

UDialogBuilderEdNode_State::UDialogBuilderEdNode_State()
{
}

void UDialogBuilderEdNode_State::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, "MultipleNode", TEXT("In"));
}

FText UDialogBuilderEdNode_State::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const UDialogBuilderNode* MyNode = Cast<UDialogBuilderNode>(NodeInstance);
	if (MyNode != NULL)
	{
		return MyNode->GetNodeTitle();
	}

	return Super::GetNodeTitle(TitleType);
}

FText UDialogBuilderEdNode_State::GetTooltipText() const
{
	const UDialogBuilderNode_State* StateNode = Cast<UDialogBuilderNode_State>(NodeInstance);
	if (StateNode)
	{
		return FText::Format(FText::FromString(TEXT("State Node\n{0}")),
			NSLOCTEXT("DialogBuilderEditor", "StateNodeScopeTooltip", "This Node signifies the conclusion point of a dialog, determining its outcome, either Completed or Failed.\nThis node contain series of events, and decorators.\n**Right click the node to start adding a subnodes."));
	}

	return Super::GetTooltipText();
}

FLinearColor UDialogBuilderEdNode_State::GetBackgroundColor() const
{
	FLinearColor Default(0.05f, 0.05f, 0.05f);
	if (UDialogBuilderNode_State* StateNode = Cast<UDialogBuilderNode_State>(NodeInstance))
	{

		switch (StateNode->DialogState)
		{
		case EDialogState::E_Complete:
			return GetDefault<UDialogBuilderSetting>()->SuccessNodeColor;
		case EDialogState::E_Fail:
			return GetDefault<UDialogBuilderSetting>()->FailedNodeColor;
		}
	}

	return GetDefault<UDialogBuilderSetting>()->RootNodeColor;
}

void UDialogBuilderEdNode_State::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	AddContextMenuActionsEvents(Menu, "DialogBuilderEdNode", Context);
	AddContextMenuActionsDecorators(Menu, "DialogBuilderEdNode", Context);
}
