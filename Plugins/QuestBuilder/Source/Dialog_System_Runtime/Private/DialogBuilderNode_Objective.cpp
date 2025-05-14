// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderNode_Objective.h"

#include "Dialog.h"
#include "DialogComponent.h"


#define LOCTEXT_NAMESPACE "DialogSystemGraphNode_TaskManager"

void UDialogBuilderNode_Objective::Initialize()
{
}


#if WITH_EDITOR
FText UDialogBuilderNode_Objective::GetNodeTitle() const
{
    return FText::FromName(ID).IsEmpty() ? FText::FromString(UDialogBuilderNode::GetShortTypeName(this)) : FText::FromName(ID);
}

void UDialogBuilderNode_Objective::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}

FText UDialogBuilderNode_Objective::GetNodeDescription() const
{
    return FText();
}
#endif

#undef LOCTEXT_NAMESPACE