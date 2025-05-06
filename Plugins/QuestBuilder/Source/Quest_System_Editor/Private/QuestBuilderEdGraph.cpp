// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderEdGraph.h"
#include "QuestBuilderEdNode_Edge.h"
#include "QuestBuilderEdNode_Root.h"
#include "QuestBuilderEdNode.h"
#include "QuestBuilderEdNode.h"
#include "QuestSystemGraphEdge.h"
#include "QuestSystemGraphNode.h"
#include "QuestSystemGraphNode_Objective.h"
#include "Condition/QuestCondition.h"
#include "Event/QuestEvent.h"
#include "Task/QuestTask.h"


UQuestBuilderEdGraph::UQuestBuilderEdGraph()
{
}

UQuestBuilderEdGraph::~UQuestBuilderEdGraph()
{
}

void UQuestBuilderEdGraph::RebuildQuestSystemGraph()
{
	//LOG_INFO(TEXT("UQuestSystemGraphEdGraph::RebuildQuestBuilder has been called"));

	UQuestSystemGraph* Graph = GetQuestSystemGraph();

	Clear();
	
	if (QuestSystem->QuestName.IsEmpty())
	{
		QuestSystem->QuestName = FText::FromName(QuestSystem->QuestID);
	}

	if (Graph->CategoryDisplayName.IsEmpty())
	{
		Graph->CategoryDisplayName = Graph->GetFName().ToString();
	}

	for (UQuestCondition* QuestCondition : QuestSystem->Conditions)
	{
		if (QuestCondition != nullptr)
		{
			QuestCondition->QuestGraph = Graph;
			QuestCondition->QuestSystem = QuestSystem;
		}
	}

	for (int i = 0; i < Nodes.Num(); ++i)
	{
		if (UQuestBuilderEdNode* EdNode = Cast<UQuestBuilderEdNode>(Nodes[i]))
		{
			if (EdNode->QuestSystemGraphNode == nullptr)
				continue;

			UQuestSystemGraphNode* QuestSystemGraphNode = EdNode->QuestSystemGraphNode;

			NodeMap.Add(QuestSystemGraphNode, EdNode);

			QuestSystem->AllNodes.Add(QuestSystemGraphNode);
			QuestSystem->NodeMap.Emplace(QuestSystemGraphNode->ID, QuestSystemGraphNode);

			for (int PinIdx = 0; PinIdx < EdNode->Pins.Num(); ++PinIdx)
			{
				UEdGraphPin* Pin = EdNode->Pins[PinIdx];

				if (Pin->Direction != EEdGraphPinDirection::EGPD_Output)
					continue;

				for (int LinkToIdx = 0; LinkToIdx < Pin->LinkedTo.Num(); ++LinkToIdx)
				{
					UQuestSystemGraphNode* ChildNode = nullptr;
					if (UQuestBuilderEdNode* EdNode_Child = Cast<UQuestBuilderEdNode>(Pin->LinkedTo[LinkToIdx]->GetOwningNode()))
					{
						ChildNode = EdNode_Child->QuestSystemGraphNode;
					}
					else if (UQuestBuilderEdNode_Edge* EdNode_Edge = Cast<UQuestBuilderEdNode_Edge>(Pin->LinkedTo[LinkToIdx]->GetOwningNode()))
					{
						UQuestBuilderEdNode* Child = EdNode_Edge->GetEndNode();
						if (Child != nullptr)
						{
							ChildNode = Child->QuestSystemGraphNode;
						}
					}

					if (ChildNode != nullptr)
					{
						QuestSystemGraphNode->ChildrenNodes.Add(ChildNode);

						ChildNode->ParentNodes.Add(QuestSystemGraphNode);
					}
					else
					{
						//LOG_ERROR(TEXT("UEdGraph_QuestBuilder::RebuildQuestBuilder can't find child node"));
					}
				}
			}

			if (UQuestSystemGraphNode_Objective* ObjectiveNode = Cast<UQuestSystemGraphNode_Objective>(QuestSystemGraphNode))
			{
				for (UQuestTask* Task : ObjectiveNode->Tasks)
				{
					if (Task != nullptr)
					{
						Task->ObjectiveNode = ObjectiveNode;
						Task->QuestGraph = Graph;
						Task->QuestSystem = QuestSystem;
					}
				}
			}

			for (UQuestCondition* QuestCondition : QuestSystemGraphNode->Conditions)
			{
				if (QuestCondition != nullptr)
				{
					QuestCondition->OwningNode = QuestSystemGraphNode;
					QuestCondition->QuestGraph = Graph;
					QuestCondition->QuestSystem = QuestSystem;
				}
			}
			
			for (UQuestEvent* QuestEvent : QuestSystemGraphNode->Events)
			{
				if (QuestEvent != nullptr)
				{
					QuestEvent->OwningNode = QuestSystemGraphNode;
					QuestEvent->QuestGraph = Graph;
					QuestEvent->QuestSystem = QuestSystem;
				}
			}

		}
		else if (UQuestBuilderEdNode_Edge* EdgeNode = Cast<UQuestBuilderEdNode_Edge>(Nodes[i]))
		{
			UQuestBuilderEdNode* StartNode = EdgeNode->GetStartNode();
			UQuestBuilderEdNode* EndNode = EdgeNode->GetEndNode();
			UQuestSystemGraphEdge* Edge = EdgeNode->QuestSystemGraphEdge;

			if (StartNode == nullptr || EndNode == nullptr || Edge == nullptr)
			{
				//LOG_ERROR(TEXT("UEdGraph_QuestBuilder::RebuildQuestBuilder add edge failed."));
				continue;
			}

			EdgeMap.Add(Edge, EdgeNode);

			Edge->Graph = Graph;
			Edge->Rename(nullptr, Graph, REN_DontCreateRedirectors | REN_DoNotDirty);
			Edge->StartNode = StartNode->QuestSystemGraphNode;
			Edge->EndNode = EndNode->QuestSystemGraphNode;
			Edge->StartNode->Edges.Add(Edge->EndNode, Edge);
		}

		if (UQuestBuilderEdNode_Root* RootNode = Cast<UQuestBuilderEdNode_Root>(Nodes[i]))
		{
			if (RootNode->QuestSystemGraphNode == nullptr)
				continue;
			QuestSystem->RootNodes.Add(RootNode->QuestSystemGraphNode);
		}

	}

	for (int i = 0; i < QuestSystem->AllNodes.Num(); ++i)
	{
		UQuestSystemGraphNode* Node = QuestSystem->AllNodes[i];
		if (Node->ParentNodes.Num() == 0)
		{
			//use this line below if you want to automatically detect rootnodes based on their parent
			//Graph->RootNodes.Add(Node);
			SortNodes(Node);
		}

		Node->QuestGraph = Graph;
		Node->QuestSystem = QuestSystem;
		Node->Rename(nullptr, Graph, REN_DontCreateRedirectors | REN_DoNotDirty);
	}

	

	QuestSystem->RootNodes.Sort([&](const UQuestSystemGraphNode& L, const UQuestSystemGraphNode& R)
	{
		UQuestBuilderEdNode* EdNode_LNode = NodeMap[&L];
		UQuestBuilderEdNode* EdNode_RNode = NodeMap[&R];
		return EdNode_LNode->NodePosX < EdNode_RNode->NodePosX;
	});

	
}

UQuestSystemGraph* UQuestBuilderEdGraph::GetQuestSystemGraph() const
{
	return CastChecked<UQuestSystemGraph>(GetOuter());
}

bool UQuestBuilderEdGraph::Modify(bool bAlwaysMarkDirty)
{
	bool Rtn = Super::Modify(bAlwaysMarkDirty);

	GetQuestSystemGraph()->Modify();

	for (int32 i = 0; i < Nodes.Num(); ++i)
	{
		Nodes[i]->Modify();
	}

	return Rtn;
}

void UQuestBuilderEdGraph::PostEditUndo()
{
	Super::PostEditUndo();

	NotifyGraphChanged();
	
}

void UQuestBuilderEdGraph::Clear()
{
	QuestSystem->ClearGraph();
	QuestSystem->NodeMap.Reset();
	NodeMap.Reset();
	EdgeMap.Reset();

	for (int i = 0; i < Nodes.Num(); ++i)
	{
		if (UQuestBuilderEdNode* EdNode = Cast<UQuestBuilderEdNode>(Nodes[i]))
		{
			UQuestSystemGraphNode* QuestGraphNode = EdNode->QuestSystemGraphNode;
			if (QuestGraphNode)
			{
				QuestGraphNode->ParentNodes.Reset();
				QuestGraphNode->ChildrenNodes.Reset();
				QuestGraphNode->Edges.Reset();
			}
		}
	}
}

void UQuestBuilderEdGraph::SortNodes(UQuestSystemGraphNode* RootNode)
{
	int Level = 0;
	TArray<UQuestSystemGraphNode*> CurrLevelNodes = { RootNode };
	TArray<UQuestSystemGraphNode*> NextLevelNodes;
	TSet<UQuestSystemGraphNode*> Visited;

	while (CurrLevelNodes.Num() != 0)
	{
		int32 LevelWidth = 0;
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
			UQuestSystemGraphNode* Node = CurrLevelNodes[i];
			Visited.Add(Node);

			auto Comp = [&](const UQuestSystemGraphNode& L, const UQuestSystemGraphNode& R)
			{
				UQuestBuilderEdNode* EdNode_LNode = NodeMap[&L];
				UQuestBuilderEdNode* EdNode_RNode = NodeMap[&R];
				return EdNode_LNode->NodePosX < EdNode_RNode->NodePosX;
			};

			Node->ChildrenNodes.Sort(Comp);
			Node->ParentNodes.Sort(Comp);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
			{
				UQuestSystemGraphNode* ChildNode = Node->ChildrenNodes[j];
				if (!Visited.Contains(ChildNode))
					NextLevelNodes.Add(Node->ChildrenNodes[j]);
			}
		}

		CurrLevelNodes = NextLevelNodes;
		NextLevelNodes.Reset();
		++Level;
	}
}
