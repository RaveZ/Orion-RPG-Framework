// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderNode_Checkpoint.h"
#include "QuestComponent.h"
#include "Quest.h"
#include "Event/OrionEvent.h"


void UQuestBuilderNode_Checkpoint::Initialize()
{
    BeginCheckpoint();
}

void UQuestBuilderNode_Checkpoint::BeginCheckpoint()
{
	//TODO : Add Delegate On Reached Checkpoint for future use
    GetQuestComponent()->UpdateQuest();
}

#if WITH_EDITOR
FText UQuestBuilderNode_Checkpoint::GetNodeTitle() const
{
    return FText::FromName(ID).IsEmpty() ? FText::FromString(UQuestBuilderNode::GetShortTypeName(this)) : FText::FromName(ID);
}
void UQuestBuilderNode_Checkpoint::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}
FText UQuestBuilderNode_Checkpoint::GetNodeDescription() const
{
	return Description;
}


FLinearColor UQuestBuilderNode_Checkpoint::GetBackgroundColor() const
{
	return FLinearColor();
}
#endif