// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "DialogBuilderNode_PlayerChoice.h"
#include "DialogBuilderGraph.h"
#include "DialogComponent.h"
#include "Event/OrionEvent.h"

UDialogBuilderNode_PlayerChoice::UDialogBuilderNode_PlayerChoice()
{
}

void UDialogBuilderNode_PlayerChoice::BeginNode()
{
	Super::BeginNode();
	EvaluateNextNode();
}

#if WITH_EDITOR
FText UDialogBuilderNode_PlayerChoice::GetNodeTitle() const
{
	return FText::FromString("Player Choice");
}

void UDialogBuilderNode_PlayerChoice::SetNodeTitle(const FText& NewTitle)
{
	ID = FName(NewTitle.ToString());
}

FText UDialogBuilderNode_PlayerChoice::GetNodeDescription() const
{
	return FText();
}

void UDialogBuilderNode_PlayerChoice::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty)
	{
		const FName PropertyName = PropertyChangedEvent.MemberProperty->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UDialogBuilderNode_PlayerChoice, ChoiceList) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(UDialogBuilderNode_PlayerChoice, ChoiceText))
		{
			OnPlayerChoiceDataChanged.Broadcast();
		}
	}
}

void UDialogBuilderNode_PlayerChoice::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.PropertyChain.GetHead() && PropertyChangedEvent.PropertyChain.GetHead()->GetValue())
	{
		const FName PropertyName = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UDialogBuilderNode_PlayerChoice, ChoiceList) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(UDialogBuilderNode_PlayerChoice, ChoiceText))
		{
			OnPlayerChoiceDataChanged.Broadcast();
		}
	}
}
#endif