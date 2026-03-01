// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderEdNode_DialogSequence.h"
#include "DialogBuilderEdNode_PlayerChoice.h"
#include "DialogBuilderNode_DialogSequence.h"
#include "DialogBuilderSetting.h"
#include "DialogBuilderNode.h"



UDialogBuilderEdNode_DialogSequence::UDialogBuilderEdNode_DialogSequence()
{
	GetOutermost()->PackageMarkedDirtyEvent.AddUObject(this, &UDialogBuilderEdNode_DialogSequence::OnPackageMarkedDirty);
}

FText UDialogBuilderEdNode_DialogSequence::GetNodeTitle(ENodeTitleType::Type TitleType) const
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

FLinearColor UDialogBuilderEdNode_DialogSequence::GetBackgroundColor() const
{
	return GetDefault<UDialogBuilderSetting>()->DialogLineNodeColor;
}

FText UDialogBuilderEdNode_DialogSequence::GetTooltipText() const
{
	const UDialogBuilderNode_DialogSequence* DialogSequenceNode = Cast<UDialogBuilderNode_DialogSequence>(NodeInstance);
	if (DialogSequenceNode)
	{
		return FText::Format(FText::FromString(TEXT("Dialog Line Node\n{0}")),
			NSLOCTEXT("DialogBuilderEditor", "SpeakerNodeScopeTooltip", "This Node contain the sequencer tailored for dialog"));
	}

	return Super::GetTooltipText();
}

void UDialogBuilderEdNode_DialogSequence::PostPasteNode()
{
	Super::PostPasteNode();
	UDialogBuilderNode_DialogSequence* DialogSequenceNode = Cast<UDialogBuilderNode_DialogSequence>(NodeInstance);
	if (DialogSequenceNode)
	{
	}
	GetGraph()->NotifyGraphChanged();
}

void UDialogBuilderEdNode_DialogSequence::PostPlacedNewNode()
{
	Super::PostPlacedNewNode();
	UDialogBuilderNode_DialogSequence* DialogSequenceNode = Cast<UDialogBuilderNode_DialogSequence>(NodeInstance);
	if (DialogSequenceNode )
	{
		DialogSequenceNode->EnsureSequenceCreated();
	}
	GetGraph()->NotifyGraphChanged();
}

void UDialogBuilderEdNode_DialogSequence::NodeConnectionListChanged()
{
	UDialogBuilderNode_DialogSequence* DialogSequenceNode = Cast<UDialogBuilderNode_DialogSequence>(NodeInstance);

	if (!DialogSequenceNode)
		return;

	for (int PinIdx = 0; PinIdx < Pins.Num(); ++PinIdx)
	{
		UEdGraphPin* Pin = Pins[PinIdx];

		if (Pin->Direction != EEdGraphPinDirection::EGPD_Output)
			continue;

		if(Pin->LinkedTo.IsValidIndex(0))
		{
			UDialogBuilderNode* ChildNode = nullptr;
			if (UDialogBuilderEdNode_PlayerChoice* PlayerOptionEdNode_Child = Cast<UDialogBuilderEdNode_PlayerChoice>(Pin->LinkedTo[0]->GetOwningNode()))
			{
				DialogSequenceNode->bIsSelector = true;
				return;
			}

		}
	}

	DialogSequenceNode->bIsSelector = false;
}

void UDialogBuilderEdNode_DialogSequence::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	AddContextMenuActionsEvents(Menu, "DialogBuilderEdNode", Context);
	AddContextMenuActionsDecorators(Menu, "DialogBuilderEdNode", Context);
}

void UDialogBuilderEdNode_DialogSequence::OnSharedParticipantChanged()
{
	GetGraph()->NotifyGraphChanged();
}

void UDialogBuilderEdNode_DialogSequence::OnPackageMarkedDirty(UPackage* ModifiedPackage, bool bWasDirty)
{
	UDialogBuilderNode_DialogSequence* DialogSequenceNode = Cast<UDialogBuilderNode_DialogSequence>(NodeInstance);
	if (DialogSequenceNode)
	{
	}
}