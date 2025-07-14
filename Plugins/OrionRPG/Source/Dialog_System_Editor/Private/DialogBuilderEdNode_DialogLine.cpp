// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderEdNode_DialogLine.h"
#include "DialogBuilderEdNode_PlayerOption.h"
#include "DialogBuilderNode_DialogLine.h"
#include "DialogBuilderSetting.h"
#include "DialogBuilderNode.h"



FText UDialogBuilderEdNode_DialogLine::GetNodeTitle(ENodeTitleType::Type TitleType) const
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

FLinearColor UDialogBuilderEdNode_DialogLine::GetBackgroundColor() const
{
	return GetDefault<UDialogBuilderSetting>()->SpeakerNodeColor;
}

FText UDialogBuilderEdNode_DialogLine::GetTooltipText() const
{
	const UDialogBuilderNode_DialogLine* DialogLineNode = Cast<UDialogBuilderNode_DialogLine>(NodeInstance);
	if (DialogLineNode)
	{
		return FText::Format(FText::FromString(TEXT("Objective Node\n{0}")),
			NSLOCTEXT("DialogBuilderEditor", "SpeakerNodeScopeTooltip", "This Node serves as the central unit within your dialog graph.\nEncapsulating a series of tasks, events, and decorators that define the player's objectives.\n**Right click the node to start adding a subnodes."));
	}

	return Super::GetTooltipText();
}

void UDialogBuilderEdNode_DialogLine::PostPasteNode()
{
	Super::PostPasteNode();
}

void UDialogBuilderEdNode_DialogLine::NodeConnectionListChanged()
{
	UDialogBuilderNode_DialogLine* DialogLineNode = Cast<UDialogBuilderNode_DialogLine>(NodeInstance);

	if (!DialogLineNode)
		return;

	for (int PinIdx = 0; PinIdx < Pins.Num(); ++PinIdx)
	{
		UEdGraphPin* Pin = Pins[PinIdx];

		if (Pin->Direction != EEdGraphPinDirection::EGPD_Output)
			continue;

		if(Pin->LinkedTo.IsValidIndex(0))
		{
			UDialogBuilderNode* ChildNode = nullptr;
			if (UDialogBuilderEdNode_PlayerOption* PlayerOptionEdNode_Child = Cast<UDialogBuilderEdNode_PlayerOption>(Pin->LinkedTo[0]->GetOwningNode()))
			{
				DialogLineNode->bIsSelector = true;
				return;
			}

		}
	}

	DialogLineNode->bIsSelector = false;
}

void UDialogBuilderEdNode_DialogLine::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	AddContextMenuActionsEvents(Menu, "DialogBuilderEdNode", Context);
	AddContextMenuActionsDecorators(Menu, "DialogBuilderEdNode", Context);
}
