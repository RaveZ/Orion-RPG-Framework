// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderNode_OptionSelector.h"
#include "DialogComponent.h"
#include "Event/OrionEvent.h"




#if WITH_EDITOR
void UDialogBuilderNode_OptionSelector::BeginNode()
{
    Super::BeginNode();
    
}

FText UDialogBuilderNode_OptionSelector::GetNodeTitle() const
{
    return FText::FromName(ID).IsEmpty() ? FText::FromString(UDialogBuilderNode::GetShortTypeName(this)) : FText::FromName(ID);
}
void UDialogBuilderNode_OptionSelector::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}
FText UDialogBuilderNode_OptionSelector::GetNodeDescription() const
{
    FString PlayerOptionDesc;


    return FText::FromString(PlayerOptionDesc);
}

#endif