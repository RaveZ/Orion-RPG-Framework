// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionInventoryEdSubNode.h"
#include "DialogBuilderSetting.h"

UOrionInventoryEdSubNode::UOrionInventoryEdSubNode()
{
	bIsReadOnly = true;
	bCanRenameNode = false;
	bIsSubNode = true;
}

void UOrionInventoryEdSubNode::AllocateDefaultPins()
{
	//subnode, no pins
}

FText UOrionInventoryEdSubNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("OrionInventoryEditor", "SubNode", "Subnode");
	
}

FLinearColor UOrionInventoryEdSubNode::GetBackgroundColor() const
{
	return GetDefault<UDialogBuilderSetting>()->SubNodeColor;
}
