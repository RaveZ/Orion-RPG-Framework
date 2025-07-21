// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderNode_PlayerLine.h"
#include "DialogBuilderGraph.h"
#include "DialogComponent.h"
#include "Event/OrionEvent.h"


UDialogBuilderNode_PlayerLine::UDialogBuilderNode_PlayerLine()
{
	bIsPlayerLine = true;
	ParticipantInfo.ParticipantTag = TAG_Dialog_Participant_Player;
}

void UDialogBuilderNode_PlayerLine::BeginNode()
{
	Super::BeginNode();
	GetOwningDialogGraph()->BeginDialogLine(this);
	GetDialogComponent()->OnDialogUpdated.Broadcast(this);
}

#if WITH_EDITOR

FText UDialogBuilderNode_PlayerLine::GetNodeTitle() const
{
	if (GetOwningDialogGraph())
	{
		return GetOwningDialogGraph()->PlayerName.IsEmpty() ? FText::FromString("Player") : GetOwningDialogGraph()->PlayerName;
	}
	return FText::FromString("");
}
void UDialogBuilderNode_PlayerLine::SetNodeTitle(const FText& NewTitle)
{
	ID = FName(NewTitle.ToString());
}

FText UDialogBuilderNode_PlayerLine::GetNodeDescription() const
{
	return DialogLineData.Line;
}

#endif