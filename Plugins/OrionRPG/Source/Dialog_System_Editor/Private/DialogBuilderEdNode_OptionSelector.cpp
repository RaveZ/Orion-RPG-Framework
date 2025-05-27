// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderEdNode_OptionSelector.h"
#include "DialogBuilderSetting.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderNode_OptionSelector.h"

UDialogBuilderEdNode_OptionSelector::UDialogBuilderEdNode_OptionSelector()
{
}

FText UDialogBuilderEdNode_OptionSelector::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const UDialogBuilderNode* MyNode = Cast<UDialogBuilderNode>(NodeInstance);
	if (MyNode != NULL)
	{
		return MyNode->GetNodeTitle();
	}

	return Super::GetNodeTitle(TitleType);
}

FText UDialogBuilderEdNode_OptionSelector::GetTooltipText() const
{
	const UDialogBuilderNode_OptionSelector* OptionSelectorNode = Cast<UDialogBuilderNode_OptionSelector>(NodeInstance);
	if (OptionSelectorNode)
	{
		return FText::Format(FText::FromString(TEXT("Option Selector Node\n{0}")),
			NSLOCTEXT("DialogBuilderEditor", "OptionSelectorNodeScopeTooltip", "This Node signifies the conclusion point of a dialog, determining its outcome, either Completed or Failed.\nThis node contain series of events, and decorators.\n**Right click the node to start adding a subnodes."));
	}

	return Super::GetTooltipText();
}

FLinearColor UDialogBuilderEdNode_OptionSelector::GetBackgroundColor() const
{
	return GetDefault<UDialogBuilderSetting>()->OptionSelectorNodeColor;
}

void UDialogBuilderEdNode_OptionSelector::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
}
