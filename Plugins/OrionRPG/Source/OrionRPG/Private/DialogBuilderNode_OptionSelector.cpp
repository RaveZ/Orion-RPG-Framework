// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderNode_OptionSelector.h"
#include "DialogComponent.h"
#include "Dialog.h"
#include "Event/OrionEvent.h"


void UDialogBuilderNode_OptionSelector::Initialize()
{
}


#if WITH_EDITOR
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


FLinearColor UDialogBuilderNode_OptionSelector::GetBackgroundColor() const
{
    FLinearColor Default(0.05f, 0.05f, 0.05f);

    return Default;
    
}
#endif