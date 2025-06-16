// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderNode_OptionSelector.h"
#include "DialogBuilderNode_PlayerOption.h"
#include "DialogComponent.h"
#include "Event/OrionEvent.h"



void UDialogBuilderNode_OptionSelector::BeginNode()
{
	Super::BeginNode();
	TArray<UDialogBuilderNode_PlayerOption*> OptionsArray;
	for (auto& Node : ChildrenNodes)
	{
		if (UDialogBuilderNode_PlayerOption* OptionNode = Cast<UDialogBuilderNode_PlayerOption>(Node))
		{
			OptionsArray.Add(OptionNode);
		}
	}

	GetDialogComponent()->OnEnterOptionSelector.Broadcast(OptionsArray);
}

void UDialogBuilderNode_OptionSelector::SelectOption(UDialogBuilderNode_PlayerOption* Option)
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

#endif