// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionInventoryEdNode_OptionSelector.h"
#include "DialogBuilderSetting.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderNode_RerouteNode.h"

UOrionInventoryEdNode_OptionSelector::UOrionInventoryEdNode_OptionSelector()
{
}

FText UOrionInventoryEdNode_OptionSelector::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const UDialogBuilderNode* MyNode = Cast<UDialogBuilderNode>(NodeInstance);
	if (MyNode != NULL)
	{
		return MyNode->GetNodeTitle();
	}

	return Super::GetNodeTitle(TitleType);
}

FText UOrionInventoryEdNode_OptionSelector::GetTooltipText() const
{
	const UDialogBuilderNode_RerouteNode* OptionSelectorNode = Cast<UDialogBuilderNode_RerouteNode>(NodeInstance);
	if (OptionSelectorNode)
	{
		return FText::Format(FText::FromString(TEXT("Option Selector Node\n{0}")),
			NSLOCTEXT("OrionInventoryEditor", "OptionSelectorNodeScopeTooltip", "This Node signifies the conclusion point of a inventory, determining its outcome, either Completed or Failed.\nThis node contain series of events, and decorators.\n**Right click the node to start adding a subnodes."));
	}

	return Super::GetTooltipText();
}

FLinearColor UOrionInventoryEdNode_OptionSelector::GetBackgroundColor() const
{
	return GetDefault<UDialogBuilderSetting>()->PlayerOptionNodeColor;
}

void UOrionInventoryEdNode_OptionSelector::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
}
