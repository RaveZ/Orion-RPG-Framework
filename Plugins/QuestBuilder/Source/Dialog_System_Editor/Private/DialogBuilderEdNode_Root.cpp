// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderEdNode_Root.h"
#include "DialogBuilderSetting.h"

UDialogBuilderEdNode_Root::UDialogBuilderEdNode_Root()
{
	bIsReadOnly = true;
	bCanRenameNode = false;
}

void UDialogBuilderEdNode_Root::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, "MultipleNode", TEXT("In"));
}

FText UDialogBuilderEdNode_Root::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("BehaviorTreeEditor", "Root", "ROOT");
	
}

FLinearColor UDialogBuilderEdNode_Root::GetBackgroundColor() const
{
	return GetDefault<UDialogBuilderSetting>()->RootNodeColor;
}
