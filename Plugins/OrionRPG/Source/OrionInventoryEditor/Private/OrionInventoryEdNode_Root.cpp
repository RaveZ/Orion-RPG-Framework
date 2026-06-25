// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionInventoryEdNode_Root.h"
#include "DialogBuilderNode_Root.h"
#include "DialogBuilderSetting.h"

UOrionInventoryEdNode_Root::UOrionInventoryEdNode_Root()
{
	bIsReadOnly = true;
	bCanRenameNode = false;
}

void UOrionInventoryEdNode_Root::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, "MultipleNode", TEXT("In"));
}

FText UOrionInventoryEdNode_Root::GetTooltipText() const
{
	const UDialogBuilderNode_Root* RootNode = Cast<UDialogBuilderNode_Root>(NodeInstance);
	if (RootNode)
	{
		return FText::Format(FText::FromString(TEXT("Inventory Start Node\n\n{0}")),
			NSLOCTEXT("OrionInventoryEditor", "RootNodeScopeTooltip", "Entry point for your inventory!"));
	}
	return Super::GetTooltipText();
}

FText UOrionInventoryEdNode_Root::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("InventoryEditor", "InventoryStart", "Inventory Start");
	
}

FLinearColor UOrionInventoryEdNode_Root::GetBackgroundColor() const
{
	return GetDefault<UDialogBuilderSetting>()->RootNodeColor;
}
