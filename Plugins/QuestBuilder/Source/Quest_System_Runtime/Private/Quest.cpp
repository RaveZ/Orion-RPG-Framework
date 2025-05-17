// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Quest.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderEdge.h"
#include "QuestBuilderNode_Objective.h"
#include "QuestBuilderNode_Root.h"


#define LOCTEXT_NAMESPACE "Quest"



UQuest::UQuest()
{
	bCanQuestBeAborted = false;
	bCanRetakeQuest = false;
	QuestState = EQuestState::E_Locked;
}

bool UQuest::Initialize(UQuestComponent* InitializingComp)
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		//We need a valid Quest component to make a quest for 
		if (!InitializingComp)
		{
			return false;
		}
	}
	return false;
}

void UQuest::Deinitialize()
{
	VisitedNodeIDs.Empty();
	CurrentNode = nullptr;
	
	for (auto& node : AllNodes)
	{
		node->Deinitialize();
	}
}

int UQuest::GetLevelNum() const
{
	int Level = 0;
	TArray<UQuestBuilderNode*> CurrLevelNodes = RootNodes;
	TArray<UQuestBuilderNode*> NextLevelNodes;

	while (CurrLevelNodes.Num() != 0)
	{
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
			UQuestBuilderNode* Node = CurrLevelNodes[i];
			check(Node != nullptr);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
			{
				NextLevelNodes.Add(Node->ChildrenNodes[j]);
			}
		}

		CurrLevelNodes = NextLevelNodes;
		NextLevelNodes.Reset();
		++Level;
	}

	return Level;
}

void UQuest::GetNodesByLevel(int Level, TArray<UQuestBuilderNode*>& Nodes)
{
	int CurrLEvel = 0;
	TArray<UQuestBuilderNode*> NextLevelNodes;

	Nodes = RootNodes;

	while (Nodes.Num() != 0)
	{
		if (CurrLEvel == Level)
			break;

		for (int i = 0; i < Nodes.Num(); ++i)
		{
			UQuestBuilderNode* Node = Nodes[i];
			check(Node != nullptr);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
			{
				NextLevelNodes.Add(Node->ChildrenNodes[j]);
			}
		}

		Nodes = NextLevelNodes;
		NextLevelNodes.Reset();
		++CurrLEvel;
	}
}

AController* UQuest::GetOwningController()
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->OwningController;
}

APawn* UQuest::GetOwningPawn()
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->OwningPawn;
}

UQuestComponent* UQuest::GetQuestComponent()
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->QuestComponent;
}

UQuestBuilderGraph* UQuest::GetOwningQuestGraph() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph;
}

void UQuest::ClearGraph()
{
	for (int i = 0; i < AllNodes.Num(); ++i)
	{
		UQuestBuilderNode* Node = AllNodes[i];
		if (Node)
		{
			Node->ParentNodes.Empty();
			Node->ChildrenNodes.Empty();
			Node->Edges.Empty();
		}
	}

	AllNodes.Empty();
	RootNodes.Empty();
}

#if WITH_EDITOR
void UQuest::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (QuestGraph)
	{
	}
}
#endif

#undef LOCTEXT_NAMESPACE