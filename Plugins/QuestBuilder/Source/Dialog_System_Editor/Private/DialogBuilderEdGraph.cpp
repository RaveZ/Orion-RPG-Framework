// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderEdGraph.h"
#include "DialogBuilderEdNode_Edge.h"
#include "DialogBuilderEdNode_Root.h"
#include "DialogBuilderEdNode.h"
#include "DialogBuilderEdNode.h"
#include "QuestBuilderEdge.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderNode_Objective.h"
#include "Decorator/OrionDecorator.h"
#include "Event/OrionEvent.h"
#include "Task/QuestTask.h"


UDialogBuilderEdGraph::UDialogBuilderEdGraph()
{
	GetOutermost()->PackageMarkedDirtyEvent.AddUObject(this, &UDialogBuilderEdGraph::OnPackageMarkedDirty);
	LastRebuildTime = 0.f;
	LastSortTime = 0.f;
}

UDialogBuilderEdGraph::~UDialogBuilderEdGraph()
{
}



void UDialogBuilderEdGraph::OnPackageMarkedDirty(UPackage* ModifiedPackage, bool bWasDirty)  
{  
   
}


void UDialogBuilderEdGraph::RebuildQuestBuilderGraph(bool bForce)
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
	
	if (Dialog->QuestName.IsEmpty())
	{
		Dialog->QuestName = FText::FromName(Dialog->ID);
	}

	if (Graph->CategoryDisplayName.IsEmpty())
	{
		Graph->CategoryDisplayName = Graph->GetFName().ToString();
	}

	for (UOrionDecorator* QuestCondition : Dialog->Conditions)
	{
		if (QuestCondition != nullptr)
		{
			QuestCondition->QuestGraph = Graph;
			QuestCondition->Quest = Dialog;
		}
	}
	CacheNodeIDMap();
	for (int i = 0; i < Nodes.Num(); ++i)
	{
		if (UDialogBuilderEdNode* QuestEdNode = Cast<UDialogBuilderEdNode>(Nodes[i]))
		{
			if (QuestEdNode->NodeInstance == nullptr)
				continue;

			UQuestBuilderNode* QuestNode = QuestEdNode ? Cast<UQuestBuilderNode>(QuestEdNode->NodeInstance) : nullptr;

			NodeMap.Add(QuestNode, QuestEdNode);

			Dialog->AllNodes.Add(QuestNode);
			Dialog->NodeMap.Emplace(QuestNode->ID, QuestNode);

			for (int PinIdx = 0; PinIdx < QuestEdNode->Pins.Num(); ++PinIdx)
			{
				UEdGraphPin* Pin = QuestEdNode->Pins[PinIdx];

				if (Pin->Direction != EEdGraphPinDirection::EGPD_Output)
					continue;

				for (int LinkToIdx = 0; LinkToIdx < Pin->LinkedTo.Num(); ++LinkToIdx)
				{
					UQuestBuilderNode* ChildNode = nullptr;
					if (UDialogBuilderEdNode* EdNode_Child = Cast<UDialogBuilderEdNode>(Pin->LinkedTo[LinkToIdx]->GetOwningNode()))
					{
						ChildNode = EdNode_Child ? Cast<UQuestBuilderNode>(EdNode_Child->NodeInstance) : nullptr;
					}
					else if (UDialogBuilderEdNode_Edge* EdNode_Edge = Cast<UDialogBuilderEdNode_Edge>(Pin->LinkedTo[LinkToIdx]->GetOwningNode()))
					{
						UDialogBuilderEdNode* Child = EdNode_Edge->GetEndNode();
						if (Child != nullptr)
						{
							ChildNode = Child ? Cast<UQuestBuilderNode>(Child->NodeInstance) : nullptr;
						}
					}

					if (ChildNode != nullptr)
					{
						QuestNode->ChildrenNodes.Add(ChildNode);

						ChildNode->ParentNodes.Add(QuestNode);
					}
				}
			}

			if (UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(QuestNode))
			{
				for (UQuestTask* Task : ObjectiveNode->Tasks)
				{
					if (Task != nullptr)
					{
						Task->ObjectiveNode = ObjectiveNode;
						Task->QuestGraph = Graph;
						Task->Quest = Dialog;
					}
				}
			}

			for (UOrionDecorator* QuestCondition : QuestNode->Conditions)
			{
				if (QuestCondition != nullptr)
				{
					QuestCondition->OwningNode = QuestNode;
					QuestCondition->QuestGraph = Graph;
					QuestCondition->Quest = Dialog;
				}
			}
			
			for (UOrionEvent* QuestEvent : QuestNode->Events)
			{
				if (QuestEvent != nullptr)
				{
					QuestEvent->OwningNode = QuestNode;
					QuestEvent->QuestGraph = Graph;
					QuestEvent->Quest = Dialog;
				}
			}

		}
		else if (UDialogBuilderEdNode_Edge* EdgeNode = Cast<UDialogBuilderEdNode_Edge>(Nodes[i]))
		{
			UDialogBuilderEdNode* StartNode = EdgeNode->GetStartNode();
			UDialogBuilderEdNode* EndNode = EdgeNode->GetEndNode();
			UQuestBuilderEdge* Edge = EdgeNode->QuestSystemGraphEdge;

			if (StartNode == nullptr || EndNode == nullptr || Edge == nullptr)
			{
				continue;
			}

			EdgeMap.Add(Edge, EdgeNode);

			Edge->Graph = Graph;
			Edge->Rename(nullptr, Graph, REN_DontCreateRedirectors | REN_DoNotDirty);
			Edge->StartNode = StartNode ? Cast<UQuestBuilderNode>(StartNode->NodeInstance) : nullptr; 
			Edge->EndNode = EndNode ? Cast<UQuestBuilderNode>(EndNode->NodeInstance) : nullptr;
			Edge->StartNode->Edges.Add(Edge->EndNode, Edge);
		}

		if (UDialogBuilderEdNode_Root* RootNode = Cast<UDialogBuilderEdNode_Root>(Nodes[i]))
		{
			if (RootNode->NodeInstance == nullptr)
				continue;
			Dialog->RootNodes.Add(Cast<UQuestBuilderNode>(RootNode->NodeInstance));
		}

	}

	for (int i = 0; i < Dialog->AllNodes.Num(); ++i)
	{
		UQuestBuilderNode* Node = Dialog->AllNodes[i];
		if (Node->ParentNodes.Num() == 0)
		{
			//use this line below if you want to automatically detect rootnodes based on their parent
			//Graph->RootNodes.Add(Node);
			SortNodes(Node);
		}

		Node->QuestGraph = Graph;
		Node->Quest = Dialog;
		Node->Rename(nullptr, Graph, REN_DontCreateRedirectors | REN_DoNotDirty);
	}

	SortAllChildrenNodes(true);
	
}

void UDialogBuilderEdGraph::SortAllChildrenNodes(bool bForce)
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
					//sort children nodes based on the PosY of QuestEdNode
					for (auto& Node : Dialog->AllNodes)
					{
						Node->ChildrenNodes.Sort([&](const UQuestBuilderNode& B, const UQuestBuilderNode& T)
							{
								UDialogBuilderEdNode* EdNode_BNode = NodeMap.FindRef(&B);
								UDialogBuilderEdNode* EdNode_TNode = NodeMap.FindRef(&T);
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

void UDialogBuilderEdGraph::CacheNodeIDMap()
{
	NodeIDMap.Reset();
	for (int i = 0; i < Nodes.Num(); ++i)
	{
		if (UDialogBuilderEdNode* QuestEdNode = Cast<UDialogBuilderEdNode>(Nodes[i]))
		{
			if (QuestEdNode->NodeInstance == nullptr)
				continue;

			UQuestBuilderNode* QuestNode = QuestEdNode ? Cast<UQuestBuilderNode>(QuestEdNode->NodeInstance) : nullptr;
			NodeIDMap.Emplace(QuestNode->ID, QuestNode);

		}
	}

	GetQuestBuilderGraph()->NodeIDMap.Append(NodeIDMap);

}

void UDialogBuilderEdGraph::AddToNodeIDMap(FName ID, UQuestBuilderNode* Node)
{
	NodeIDMap.Emplace(ID, Node);
	GetQuestBuilderGraph()->NodeIDMap.Emplace(ID, Node);
}

UQuestBuilderGraph* UDialogBuilderEdGraph::GetQuestBuilderGraph() const
{
	return CastChecked<UQuestBuilderGraph>(GetOuter());
}

bool UDialogBuilderEdGraph::Modify(bool bAlwaysMarkDirty)
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
void UDialogBuilderEdGraph::PostEditUndo()
{
	Super::PostEditUndo();

	NotifyGraphChanged();
	RebuildQuestBuilderGraph();
}
#endif // WITH_EDITOR

void UDialogBuilderEdGraph::Clear()
{
	Dialog->ClearGraph();
	Dialog->NodeMap.Reset();
	NodeMap.Reset();
	EdgeMap.Reset();

	for (int i = 0; i < Nodes.Num(); ++i)
	{
		if (UDialogBuilderEdNode* QuestEdNode = Cast<UDialogBuilderEdNode>(Nodes[i]))
		{
			UQuestBuilderNode* QuestNode = QuestEdNode ? Cast<UQuestBuilderNode>(QuestEdNode->NodeInstance) : nullptr;
			if (QuestNode)
			{
				QuestNode->ParentNodes.Reset();
				QuestNode->ChildrenNodes.Reset();
				QuestNode->Edges.Reset();
			}
		}
	}
}

void UDialogBuilderEdGraph::SortNodes(UQuestBuilderNode* RootNode)
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
				UDialogBuilderEdNode* EdNode_LNode = NodeMap[&L];
				UDialogBuilderEdNode* EdNode_RNode = NodeMap[&R];
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
