// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderNode_Speaker.h"
#include "DialogComponent.h"


#define LOCTEXT_NAMESPACE "DialogSystemGraphNode"



#if WITH_EDITOR
FText UDialogBuilderNode_Speaker::GetNodeTitle() const
{
    return FText::FromName(ID).IsEmpty() ? FText::FromString(UDialogBuilderNode::GetShortTypeName(this)) : FText::FromName(ID);
}

void UDialogBuilderNode_Speaker::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}

FText UDialogBuilderNode_Speaker::GetNodeDescription() const
{
    return Description;
}
#endif

#undef LOCTEXT_NAMESPACE