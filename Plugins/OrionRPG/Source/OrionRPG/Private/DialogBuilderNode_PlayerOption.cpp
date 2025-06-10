// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderNode_PlayerOption.h"
#include "DialogComponent.h"
#include "Event/OrionEvent.h"



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
    FString PlayerOptionDesc;


	return FText::FromString(PlayerOptionDesc);
}

#endif