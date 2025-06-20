// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderNode_PlayerOption.h"
#include "DialogComponent.h"
#include "Event/OrionEvent.h"


UDialogBuilderNode_PlayerOption::UDialogBuilderNode_PlayerOption()
{
}

void UDialogBuilderNode_PlayerOption::BeginNode()
{
	Super::BeginNode();
}

#if WITH_EDITOR

FText UDialogBuilderNode_PlayerOption::GetNodeTitle() const
{
    return FText::FromName(ID).IsEmpty() ? FText::FromString(UDialogBuilderNode::GetShortTypeName(this)) : FText::FromName(ID);
}
void UDialogBuilderNode_PlayerOption::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}
FText UDialogBuilderNode_PlayerOption::GetNodeDescription() const
{
	return DialogSequenceData.Line;
}

#endif