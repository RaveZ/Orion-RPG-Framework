// Copyright 2024 Ivan Chandra. All Rights Reserved.

#include "QuestNodeWidget.h"


void UQuestNodeWidget::InitializeFromNode(UQuestSystemGraphNode* InNode, UQuestSystem* InQuest)
{
	if (InNode)
	{
		Node = InNode;
		Quest = InQuest;

		OnNodeInitialized(InNode, InQuest);
	}
}
