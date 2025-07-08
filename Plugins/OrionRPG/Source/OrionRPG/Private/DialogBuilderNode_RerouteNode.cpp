// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderNode_RerouteNode.h"
#include "DialogBuilderNode_PlayerOption.h"
#include "DialogComponent.h"
#include "Event/OrionEvent.h"



void UDialogBuilderNode_RerouteNode::BeginNode()
{
	Super::BeginNode();
	TArray<UDialogBuilderNode_PlayerOption*> PlayerOptions;
	for (auto& Node : ChildrenNodes)
	{
		if (UDialogBuilderNode_PlayerOption* OptionNode = Cast<UDialogBuilderNode_PlayerOption>(Node))
		{
			PlayerOptions.Add(OptionNode);
		}
	}

	GetDialogComponent()->OnEnterOptionSelection.Broadcast(PlayerOptions);
}

void UDialogBuilderNode_RerouteNode::SelectOption(UDialogBuilderNode_PlayerOption* Option)
{

}

#if WITH_EDITOR
FText UDialogBuilderNode_RerouteNode::GetNodeTitle() const
{
    return FText::FromName(ID).IsEmpty() ? FText::FromString(UDialogBuilderNode::GetShortTypeName(this)) : FText::FromName(ID);
}
void UDialogBuilderNode_RerouteNode::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}
FText UDialogBuilderNode_RerouteNode::GetNodeDescription() const
{
    FString PlayerOptionDesc;


    return FText::FromString(PlayerOptionDesc);
}

#endif