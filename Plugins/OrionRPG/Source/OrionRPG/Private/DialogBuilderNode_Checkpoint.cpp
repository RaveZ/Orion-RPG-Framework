// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderNode_Checkpoint.h"
#include "DialogComponent.h"
#include "Dialog.h"
#include "Event/OrionEvent.h"


void UDialogBuilderNode_Checkpoint::Initialize()
{
    BeginCheckpoint();
}

void UDialogBuilderNode_Checkpoint::BeginCheckpoint()
{
	//TODO : Add Delegate On Reached Checkpoint for future use
    GetDialogComponent()->UpdateDialog();
}

#if WITH_EDITOR
FText UDialogBuilderNode_Checkpoint::GetNodeTitle() const
{
    return FText::FromName(ID).IsEmpty() ? FText::FromString(UDialogBuilderNode::GetShortTypeName(this)) : FText::FromName(ID);
}
void UDialogBuilderNode_Checkpoint::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}
FText UDialogBuilderNode_Checkpoint::GetNodeDescription() const
{
	return Description;
}


FLinearColor UDialogBuilderNode_Checkpoint::GetBackgroundColor() const
{
	return FLinearColor();
}
#endif