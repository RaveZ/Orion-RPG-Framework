// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderEdNode_Checkpoint.h"
#include "DialogBuilderSetting.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderNode_Checkpoint.h"

UDialogBuilderEdNode_Checkpoint::UDialogBuilderEdNode_Checkpoint()
{
}


FText UDialogBuilderEdNode_Checkpoint::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const UDialogBuilderNode* MyNode = Cast<UDialogBuilderNode>(NodeInstance);
	if (MyNode != NULL)
	{
		return MyNode->GetNodeTitle();
	}

	return Super::GetNodeTitle(TitleType);
}

FText UDialogBuilderEdNode_Checkpoint::GetTooltipText() const
{
	const UDialogBuilderNode_Checkpoint* CheckpointNode = Cast<UDialogBuilderNode_Checkpoint>(NodeInstance);
	if (CheckpointNode)
	{
		return FText::Format(FText::FromString(TEXT("Checkpoint Node\n{0}")),
			NSLOCTEXT("DialogBuilderEditor", "StateNodeScopeTooltip", "This Node serves as the checkpoint for the dialog.\nUseful if we want to know that some dialog has reached a certain checkpoint.\n**Right click the node to start adding a subnodes."));
	}

	return Super::GetTooltipText();
}

FLinearColor UDialogBuilderEdNode_Checkpoint::GetBackgroundColor() const
{
	return GetDefault<UDialogBuilderSetting>()->CheckpointNodeColor;
}

void UDialogBuilderEdNode_Checkpoint::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	AddContextMenuActionsEvents(Menu, "DialogBuilderEdNode", Context);
	AddContextMenuActionsDecorators(Menu, "DialogBuilderEdNode", Context);
}
