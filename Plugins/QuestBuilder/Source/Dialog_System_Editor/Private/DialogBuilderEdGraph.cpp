// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderEdGraph.h"
#include "DialogBuilderEdNode_Edge.h"
#include "DialogBuilderEdNode_Root.h"
#include "DialogBuilderEdNode.h"
#include "DialogBuilderEdNode.h"
#include "DialogBuilderEdge.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderNode_Objective.h"
#include "Decorator/OrionDecorator.h"
#include "Event/OrionEvent.h"


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


void UDialogBuilderEdGraph::RebuildDialogBuilderGraph(bool bForce)
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

	UE_LOG(LogTemp, Log, TEXT("RebuildDialogBuilderGraph started."));

	UDialogBuilderGraph* Graph = GetDialogBuilderGraph();

	Clear();
	
	if (Dialog->DialogName.IsEmpty())
	{
		Dialog->DialogName = FText::FromName(Dialog->ID);
	}

	if (Graph->CategoryDisplayName.IsEmpty())
	{
		Graph->CategoryDisplayName = Graph->GetFName().ToString();
	}

	/*for (UOrionDecorator* DialogCondition : Dialog->Decorators)
	{
		if (DialogCondition != nullptr)
		{
			DialogCondition->QuestGraph = Graph;
		}
	}*/
	CacheNodeIDMap();
	for (int i = 0; i < Nodes.Num(); ++i)
	{
		if (UDialogBuilderEdNode* DialogEdNode = Cast<UDialogBuilderEdNode>(Nodes[i]))
		{
			if (DialogEdNode->NodeInstance == nullptr)
				continue;

			UDialogBuilderNode* DialogNode = DialogEdNode ? Cast<UDialogBuilderNode>(DialogEdNode->NodeInstance) : nullptr;

			NodeMap.Add(DialogNode, DialogEdNode);

			Dialog->AllNodes.Add(DialogNode);
			Dialog->NodeMap.Emplace(DialogNode->ID, DialogNode);

			for (int PinIdx = 0; PinIdx < DialogEdNode->Pins.Num(); ++PinIdx)
			{
				UEdGraphPin* Pin = DialogEdNode->Pins[PinIdx];

				if (Pin->Direction != EEdGraphPinDirection::EGPD_Output)
					continue;

				for (int LinkToIdx = 0; LinkToIdx < Pin->LinkedTo.Num(); ++LinkToIdx)
				{
					UDialogBuilderNode* ChildNode = nullptr;
					if (UDialogBuilderEdNode* EdNode_Child = Cast<UDialogBuilderEdNode>(Pin->LinkedTo[LinkToIdx]->GetOwningNode()))
					{
						ChildNode = EdNode_Child ? Cast<UDialogBuilderNode>(EdNode_Child->NodeInstance) : nullptr;
					}
					else if (UDialogBuilderEdNode_Edge* EdNode_Edge = Cast<UDialogBuilderEdNode_Edge>(Pin->LinkedTo[LinkToIdx]->GetOwningNode()))
					{
						UDialogBuilderEdNode* Child = EdNode_Edge->GetEndNode();
						if (Child != nullptr)
						{
							ChildNode = Child ? Cast<UDialogBuilderNode>(Child->NodeInstance) : nullptr;
						}
					}

					if (ChildNode != nullptr)
					{
						DialogNode->ChildrenNodes.Add(ChildNode);

						ChildNode->ParentNodes.Add(DialogNode);
					}
				}
			}


			/*for (UOrionDecorator* DialogCondition : DialogNode->Decorators)
			{
				if (DialogCondition != nullptr)
				{
					DialogCondition->OwningNode = DialogNode;
					DialogCondition->DialogGraph = Graph;
					DialogCondition->Dialog = Dialog;
				}
			}
			
			for (UOrionEvent* DialogEvent : DialogNode->Events)
			{
				if (DialogEvent != nullptr)
				{
					DialogEvent->OwningNode = DialogNode;
					DialogEvent->DialogGraph = Graph;
					DialogEvent->Dialog = Dialog;
				}
			}*/

		}
		else if (UDialogBuilderEdNode_Edge* EdgeNode = Cast<UDialogBuilderEdNode_Edge>(Nodes[i]))
		{
			UDialogBuilderEdNode* StartNode = EdgeNode->GetStartNode();
			UDialogBuilderEdNode* EndNode = EdgeNode->GetEndNode();
			UDialogBuilderEdge* Edge = EdgeNode->DialogSystemGraphEdge;

			if (StartNode == nullptr || EndNode == nullptr || Edge == nullptr)
			{
				continue;
			}

			EdgeMap.Add(Edge, EdgeNode);

			Edge->Graph = Graph;
			Edge->Rename(nullptr, Graph, REN_DontCreateRedirectors | REN_DoNotDirty);
			Edge->StartNode = StartNode ? Cast<UDialogBuilderNode>(StartNode->NodeInstance) : nullptr; 
			Edge->EndNode = EndNode ? Cast<UDialogBuilderNode>(EndNode->NodeInstance) : nullptr;
			Edge->StartNode->Edges.Add(Edge->EndNode, Edge);
		}

		if (UDialogBuilderEdNode_Root* RootNode = Cast<UDialogBuilderEdNode_Root>(Nodes[i]))
		{
			if (RootNode->NodeInstance == nullptr)
				continue;
			Dialog->RootNodes.Add(Cast<UDialogBuilderNode>(RootNode->NodeInstance));
		}

	}

	for (int i = 0; i < Dialog->AllNodes.Num(); ++i)
	{
		UDialogBuilderNode* Node = Dialog->AllNodes[i];
		if (Node->ParentNodes.Num() == 0)
		{
			//use this line below if you want to automatically detect rootnodes based on their parent
			//Graph->RootNodes.Add(Node);
			SortNodes(Node);
		}

		Node->DialogGraph = Graph;
		Node->Dialog = Dialog;
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
					//sort children nodes based on the PosY of DialogEdNode
					for (auto& Node : Dialog->AllNodes)
					{
						Node->ChildrenNodes.Sort([&](const UDialogBuilderNode& B, const UDialogBuilderNode& T)
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
		if (UDialogBuilderEdNode* DialogEdNode = Cast<UDialogBuilderEdNode>(Nodes[i]))
		{
			if (DialogEdNode->NodeInstance == nullptr)
				continue;

			UDialogBuilderNode* DialogNode = DialogEdNode ? Cast<UDialogBuilderNode>(DialogEdNode->NodeInstance) : nullptr;
			NodeIDMap.Emplace(DialogNode->ID, DialogNode);

		}
	}

	GetDialogBuilderGraph()->NodeIDMap.Append(NodeIDMap);

}

void UDialogBuilderEdGraph::AddToNodeIDMap(FName ID, UDialogBuilderNode* Node)
{
	NodeIDMap.Emplace(ID, Node);
	GetDialogBuilderGraph()->NodeIDMap.Emplace(ID, Node);
}

UDialogBuilderGraph* UDialogBuilderEdGraph::GetDialogBuilderGraph() const
{
	return CastChecked<UDialogBuilderGraph>(GetOuter());
}

bool UDialogBuilderEdGraph::Modify(bool bAlwaysMarkDirty)
{
	bool Rtn = Super::Modify(bAlwaysMarkDirty);

	GetDialogBuilderGraph()->Modify();

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
	RebuildDialogBuilderGraph();
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
		if (UDialogBuilderEdNode* DialogEdNode = Cast<UDialogBuilderEdNode>(Nodes[i]))
		{
			UDialogBuilderNode* DialogNode = DialogEdNode ? Cast<UDialogBuilderNode>(DialogEdNode->NodeInstance) : nullptr;
			if (DialogNode)
			{
				DialogNode->ParentNodes.Reset();
				DialogNode->ChildrenNodes.Reset();
				DialogNode->Edges.Reset();
			}
		}
	}
}

void UDialogBuilderEdGraph::SortNodes(UDialogBuilderNode* RootNode)
{
	int Level = 0;
	TArray<UDialogBuilderNode*> CurrLevelNodes = { RootNode };
	TArray<UDialogBuilderNode*> NextLevelNodes;
	TSet<UDialogBuilderNode*> Visited;

	while (CurrLevelNodes.Num() != 0)
	{
		int32 LevelWidth = 0;
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
			UDialogBuilderNode* Node = CurrLevelNodes[i];
			Visited.Add(Node);

			auto Comp = [&](const UDialogBuilderNode& L, const UDialogBuilderNode& R)
			{
				UDialogBuilderEdNode* EdNode_LNode = NodeMap[&L];
				UDialogBuilderEdNode* EdNode_RNode = NodeMap[&R];
				return EdNode_LNode->NodePosX < EdNode_RNode->NodePosX;
			};

			Node->ChildrenNodes.Sort(Comp);
			Node->ParentNodes.Sort(Comp);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
			{
				UDialogBuilderNode* ChildNode = Node->ChildrenNodes[j];
				if (!Visited.Contains(ChildNode))
					NextLevelNodes.Add(Node->ChildrenNodes[j]);
			}
		}

		CurrLevelNodes = NextLevelNodes;
		NextLevelNodes.Reset();
		++Level;
	}
}
