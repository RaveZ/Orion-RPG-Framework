// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderEdGraph.h"
#include "QuestBuilderEdNode_Edge.h"
#include "QuestBuilderEdNode_Root.h"
#include "QuestBuilderEdNode.h"
#include "QuestBuilderEdNode.h"
#include "QuestBuilderEdge.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderNode_Objective.h"
#include "Condition/QuestCondition.h"
#include "Event/QuestEvent.h"
#include "Task/QuestTask.h"


UQuestBuilderEdGraph::UQuestBuilderEdGraph()
{
	GetOutermost()->PackageMarkedDirtyEvent.AddUObject(this, &UQuestBuilderEdGraph::OnPackageMarkedDirty);
	LastRebuildTime = 0.f;
	LastSortTime = 0.f;
}

UQuestBuilderEdGraph::~UQuestBuilderEdGraph()
{
}



void UQuestBuilderEdGraph::OnPackageMarkedDirty(UPackage* ModifiedPackage, bool bWasDirty)  
{  
   
}


void UQuestBuilderEdGraph::RebuildQuestBuilderGraph(bool bForce)
{
	const double CurrentTime = FPlatformTime::Seconds();

	if (!bForce)
	{
		// Prevent the function from being called twice in the same timeframe (e.g., 1 second)
		if (CurrentTime - LastRebuildTime < .5f)
		{
			return;
		}
	}

	LastRebuildTime = CurrentTime;

	UE_LOG(LogTemp, Log, TEXT("RebuildQuestBuilderGraph started."));

	UQuestBuilderGraph* Graph = GetQuestBuilderGraph();

	Clear();
	
	if (Quest->QuestName.IsEmpty())
	{
		Quest->QuestName = FText::FromName(Quest->ID);
	}

	if (Graph->CategoryDisplayName.IsEmpty())
	{
		Graph->CategoryDisplayName = Graph->GetFName().ToString();
	}

	for (UQuestCondition* QuestCondition : Quest->Conditions)
	{
		if (QuestCondition != nullptr)
		{
			QuestCondition->QuestGraph = Graph;
			QuestCondition->Quest = Quest;
		}
	}
	CacheNodeIDMap();
	for (int i = 0; i < Nodes.Num(); ++i)
	{
		if (UQuestBuilderEdNode* EdNode = Cast<UQuestBuilderEdNode>(Nodes[i]))
		{
			if (EdNode->QuestBuilderNode == nullptr)
				continue;

			UQuestBuilderNode* QuestBuilderNode = EdNode->QuestBuilderNode;

			NodeMap.Add(QuestBuilderNode, EdNode);

			Quest->AllNodes.Add(QuestBuilderNode);
			Quest->NodeMap.Emplace(QuestBuilderNode->ID, QuestBuilderNode);

			for (int PinIdx = 0; PinIdx < EdNode->Pins.Num(); ++PinIdx)
			{
				UEdGraphPin* Pin = EdNode->Pins[PinIdx];

				if (Pin->Direction != EEdGraphPinDirection::EGPD_Output)
					continue;

				for (int LinkToIdx = 0; LinkToIdx < Pin->LinkedTo.Num(); ++LinkToIdx)
				{
					UQuestBuilderNode* ChildNode = nullptr;
					if (UQuestBuilderEdNode* EdNode_Child = Cast<UQuestBuilderEdNode>(Pin->LinkedTo[LinkToIdx]->GetOwningNode()))
					{
						ChildNode = EdNode_Child->QuestBuilderNode;
					}
					else if (UQuestBuilderEdNode_Edge* EdNode_Edge = Cast<UQuestBuilderEdNode_Edge>(Pin->LinkedTo[LinkToIdx]->GetOwningNode()))
					{
						UQuestBuilderEdNode* Child = EdNode_Edge->GetEndNode();
						if (Child != nullptr)
						{
							ChildNode = Child->QuestBuilderNode;
						}
					}

					if (ChildNode != nullptr)
					{
						QuestBuilderNode->ChildrenNodes.Add(ChildNode);

						ChildNode->ParentNodes.Add(QuestBuilderNode);
					}
				}
			}

			if (UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(QuestBuilderNode))
			{
				for (UQuestTask* Task : ObjectiveNode->Tasks)
				{
					if (Task != nullptr)
					{
						Task->ObjectiveNode = ObjectiveNode;
						Task->QuestGraph = Graph;
						Task->Quest = Quest;
					}
				}
			}

			for (UQuestCondition* QuestCondition : QuestBuilderNode->Conditions)
			{
				if (QuestCondition != nullptr)
				{
					QuestCondition->OwningNode = QuestBuilderNode;
					QuestCondition->QuestGraph = Graph;
					QuestCondition->Quest = Quest;
				}
			}
			
			for (UQuestEvent* QuestEvent : QuestBuilderNode->Events)
			{
				if (QuestEvent != nullptr)
				{
					QuestEvent->OwningNode = QuestBuilderNode;
					QuestEvent->QuestGraph = Graph;
					QuestEvent->Quest = Quest;
				}
			}

		}
		else if (UQuestBuilderEdNode_Edge* EdgeNode = Cast<UQuestBuilderEdNode_Edge>(Nodes[i]))
		{
			UQuestBuilderEdNode* StartNode = EdgeNode->GetStartNode();
			UQuestBuilderEdNode* EndNode = EdgeNode->GetEndNode();
			UQuestBuilderEdge* Edge = EdgeNode->QuestSystemGraphEdge;

			if (StartNode == nullptr || EndNode == nullptr || Edge == nullptr)
			{
				continue;
			}

			EdgeMap.Add(Edge, EdgeNode);

			Edge->Graph = Graph;
			Edge->Rename(nullptr, Graph, REN_DontCreateRedirectors | REN_DoNotDirty);
			Edge->StartNode = StartNode->QuestBuilderNode;
			Edge->EndNode = EndNode->QuestBuilderNode;
			Edge->StartNode->Edges.Add(Edge->EndNode, Edge);
		}

		if (UQuestBuilderEdNode_Root* RootNode = Cast<UQuestBuilderEdNode_Root>(Nodes[i]))
		{
			if (RootNode->QuestBuilderNode == nullptr)
				continue;
			Quest->RootNodes.Add(RootNode->QuestBuilderNode);
		}

	}

	for (int i = 0; i < Quest->AllNodes.Num(); ++i)
	{
		UQuestBuilderNode* Node = Quest->AllNodes[i];
		if (Node->ParentNodes.Num() == 0)
		{
			//use this line below if you want to automatically detect rootnodes based on their parent
			//Graph->RootNodes.Add(Node);
			SortNodes(Node);
		}

		Node->QuestGraph = Graph;
		Node->Quest = Quest;
		Node->Rename(nullptr, Graph, REN_DontCreateRedirectors | REN_DoNotDirty);
	}

	SortAllChildrenNodes(true);
	
}

void UQuestBuilderEdGraph::SortAllChildrenNodes(bool bForce)
{
	const double CurrentTime = FPlatformTime::Seconds();

	if (!bForce)
	{
		// Prevent the function from being called twice in the same timeframe (e.g., 1 second)
		if (CurrentTime - LastSortTime < .5f)
		{
			return;
		}
	}

	LastSortTime = CurrentTime;
	FTimerHandle TimerHandle;
	if (GEditor)
	{
		GEditor->GetTimerManager()->SetTimer(
			TimerHandle,
			FTimerDelegate::CreateLambda([this]()
				{
					//sort children nodes based on the PosY of EdNode
					for (auto& Node : Quest->AllNodes)
					{
						Node->ChildrenNodes.Sort([&](const UQuestBuilderNode& B, const UQuestBuilderNode& T)
							{
								UQuestBuilderEdNode* EdNode_BNode = NodeMap.FindRef(&B);
								UQuestBuilderEdNode* EdNode_TNode = NodeMap.FindRef(&T);
								if (EdNode_BNode && EdNode_TNode)
								{
									return EdNode_TNode->NodePosY > EdNode_BNode->NodePosY;
								}
								else
								{
									return false;
								}
							});

					}
				}),
			.5f, // Delay in seconds
			false // Do not loop
		);
	}
}

void UQuestBuilderEdGraph::CacheNodeIDMap()
{
	NodeIDMap.Reset();
	for (int i = 0; i < Nodes.Num(); ++i)
	{
		if (UQuestBuilderEdNode* EdNode = Cast<UQuestBuilderEdNode>(Nodes[i]))
		{
			if (EdNode->QuestBuilderNode == nullptr)
				continue;

			UQuestBuilderNode* QuestBuilderNode = EdNode->QuestBuilderNode;
			NodeIDMap.Emplace(QuestBuilderNode->ID, QuestBuilderNode);

		}
	}

	GetQuestBuilderGraph()->NodeIDMap.Append(NodeIDMap);

}

void UQuestBuilderEdGraph::AddToNodeIDMap(FName ID, UQuestBuilderNode* Node)
{
	NodeIDMap.Emplace(ID, Node);
	GetQuestBuilderGraph()->NodeIDMap.Emplace(ID, Node);
}

UQuestBuilderGraph* UQuestBuilderEdGraph::GetQuestBuilderGraph() const
{
	return CastChecked<UQuestBuilderGraph>(GetOuter());
}

bool UQuestBuilderEdGraph::Modify(bool bAlwaysMarkDirty)
{
	bool Rtn = Super::Modify(bAlwaysMarkDirty);

	GetQuestBuilderGraph()->Modify();

	for (int32 i = 0; i < Nodes.Num(); ++i)
	{
		Nodes[i]->Modify();
	}

	return Rtn;
}

#if WITH_EDITOR
void UQuestBuilderEdGraph::PostEditUndo()
{
	Super::PostEditUndo();

	NotifyGraphChanged();
	RebuildQuestBuilderGraph();
}
#endif // WITH_EDITOR

void UQuestBuilderEdGraph::Clear()
{
	Quest->ClearGraph();
	Quest->NodeMap.Reset();
	NodeMap.Reset();
	EdgeMap.Reset();

	for (int i = 0; i < Nodes.Num(); ++i)
	{
		if (UQuestBuilderEdNode* EdNode = Cast<UQuestBuilderEdNode>(Nodes[i]))
		{
			UQuestBuilderNode* QuestGraphNode = EdNode->QuestBuilderNode;
			if (QuestGraphNode)
			{
				QuestGraphNode->ParentNodes.Reset();
				QuestGraphNode->ChildrenNodes.Reset();
				QuestGraphNode->Edges.Reset();
			}
		}
	}
}

void UQuestBuilderEdGraph::SortNodes(UQuestBuilderNode* RootNode)
{
	int Level = 0;
	TArray<UQuestBuilderNode*> CurrLevelNodes = { RootNode };
	TArray<UQuestBuilderNode*> NextLevelNodes;
	TSet<UQuestBuilderNode*> Visited;

	while (CurrLevelNodes.Num() != 0)
	{
		int32 LevelWidth = 0;
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
			UQuestBuilderNode* Node = CurrLevelNodes[i];
			Visited.Add(Node);

			auto Comp = [&](const UQuestBuilderNode& L, const UQuestBuilderNode& R)
			{
				UQuestBuilderEdNode* EdNode_LNode = NodeMap[&L];
				UQuestBuilderEdNode* EdNode_RNode = NodeMap[&R];
				return EdNode_LNode->NodePosX < EdNode_RNode->NodePosX;
			};

			Node->ChildrenNodes.Sort(Comp);
			Node->ParentNodes.Sort(Comp);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
			{
				UQuestBuilderNode* ChildNode = Node->ChildrenNodes[j];
				if (!Visited.Contains(ChildNode))
					NextLevelNodes.Add(Node->ChildrenNodes[j]);
			}
		}

		CurrLevelNodes = NextLevelNodes;
		NextLevelNodes.Reset();
		++Level;
	}
}
