// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionInventoryEdNode_Speaker.h"
#include "DialogBuilderNode_DialogLine.h"
#include "DialogBuilderSetting.h"
#include "DialogBuilderNode.h"

FText UOrionInventoryEdNode_Speaker::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const UDialogBuilderNode* InventoryNode = Cast<UDialogBuilderNode>(NodeInstance);

	if (InventoryNode != NULL)
	{
		return InventoryNode->GetNodeTitle();
	}
	else if (!ClassData.GetClassName().IsEmpty())
	{
		FString StoredClassName = ClassData.GetClassName();
		StoredClassName.RemoveFromEnd(TEXT("_C"));

		return FText::Format(NSLOCTEXT("InventoryGraph", "NodeClassError", "Class {0} not found, make sure it's saved!"), FText::FromString(StoredClassName));
	}

	return Super::GetNodeTitle(TitleType);
}

FLinearColor UOrionInventoryEdNode_Speaker::GetBackgroundColor() const
{
	return GetDefault<UDialogBuilderSetting>()->SpeakerNodeColor;
}

FText UOrionInventoryEdNode_Speaker::GetTooltipText() const
{
	const UDialogBuilderNode_DialogLine* SpeakerNode = Cast<UDialogBuilderNode_DialogLine>(NodeInstance);
	if (SpeakerNode)
	{
		return FText::Format(FText::FromString(TEXT("Objective Node\n{0}")),
			NSLOCTEXT("OrionInventoryEditor", "SpeakerNodeScopeTooltip", "This Node serves as the central unit within your inventory graph.\nEncapsulating a series of tasks, events, and decorators that define the player's objectives.\n**Right click the node to start adding a subnodes."));
	}

	return Super::GetTooltipText();
}

void UOrionInventoryEdNode_Speaker::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	AddContextMenuActionsEvents(Menu, "OrionInventoryEdNode", Context);
	AddContextMenuActionsDecorators(Menu, "OrionInventoryEdNode", Context);
}
