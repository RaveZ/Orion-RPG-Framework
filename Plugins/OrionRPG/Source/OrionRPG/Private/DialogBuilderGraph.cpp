// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderGraph.h"
#include "DialogComponent.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderEdge.h"
#include "DialogBuilderNode_Root.h"

#define LOCTEXT_NAMESPACE "DialogBuilderGraph"

UDialogBuilderGraph::UDialogBuilderGraph()
{
}

UDialogBuilderGraph::~UDialogBuilderGraph()
{
	
}

bool UDialogBuilderGraph::Initialize(UDialogComponent* InitializingComp)
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		//We need a valid Quest component to make a quest for 
		if (!InitializingComp)
		{
			return false;
		}
		OwningController = InitializingComp->GetOwningController();
		DialogComponent = InitializingComp;
		VisitedNodeIDs.Empty();
		NodeMap.Empty();
		for (auto& Node : AllNodes)
		{
			if (Node)
			{
				Node->DialogGraph = this;
				Node->OwningController = InitializingComp->GetOwningController();
				Node->DialogComponent = InitializingComp;
				NodeMap.Emplace(Node->ID, Node);
			}
		}
	}
	return false;
}

void UDialogBuilderGraph::Deinitialize()
{
	VisitedNodeIDs.Empty();
	CurrentNode = nullptr;
	
	for (auto& Node : AllNodes)
	{
		Node->Deinitialize();
	}
}

void UDialogBuilderGraph::BeginNode(UDialogBuilderNode* InNode, bool bLaunchEventOnLoad)
{
	if (InNode)
	{
		CurrentNode = InNode;
	}
	if (InNode == nullptr)  
	{  
		UE_LOG(LogTemp, Warning, TEXT("BeginNode: Node is null for Dialog ID: %s"), *ID.ToString());  
		return;  
	}
	
	if (UDialogBuilderNode_Root* RootNode = Cast<UDialogBuilderNode_Root>(InNode))
	{
		RootNode->Initialize();
		return;
	}
	
	//add visited node
	VisitedNodeIDs.AddUnique(InNode->ID);
	
	//InitializeNode
	InNode->Initialize(bLaunchEventOnLoad);
}

int UDialogBuilderGraph::GetLevelNum() const
{
	int Level = 0;
	TArray<UDialogBuilderNode*> CurrLevelNodes = RootNodes;
	TArray<UDialogBuilderNode*> NextLevelNodes;

	while (CurrLevelNodes.Num() != 0)
	{
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
			UDialogBuilderNode* Node = CurrLevelNodes[i];
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

void UDialogBuilderGraph::GetNodesByLevel(int Level, TArray<UDialogBuilderNode*>& Nodes)
{
	int CurrLEvel = 0;
	TArray<UDialogBuilderNode*> NextLevelNodes;

	Nodes = RootNodes;

	while (Nodes.Num() != 0)
	{
		if (CurrLEvel == Level)
			break;

		for (int i = 0; i < Nodes.Num(); ++i)
		{
			UDialogBuilderNode* Node = Nodes[i];
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

void UDialogBuilderGraph::ClearGraph()
{
	for (int i = 0; i < AllNodes.Num(); ++i)
	{
		UDialogBuilderNode* Node = AllNodes[i];
		if (Node)
		{
			Node->ParentNodes.Empty();
			Node->ChildrenNodes.Empty();
			Node->Edges.Empty();
		}
	}

	VisitedNodeIDs.Empty();
	AllNodes.Empty();
	RootNodes.Empty();
	NodeMap.Empty();
}

AController* UDialogBuilderGraph::GetOwningController()
{
	return OwningController;
}

APawn* UDialogBuilderGraph::GetOwningPawn()
{
	if (DialogComponent)
	{
		return DialogComponent->GetOwningPawn();
	}
	return nullptr;
}

UDialogComponent* UDialogBuilderGraph::GetDialogComponent()
{
	return DialogComponent;
}

#if WITH_EDITORONLY_DATA

void UDialogBuilderGraph::GetAllGraphs(TArray<UEdGraph*>& Graphs) const
{
	

	for (int32 i = 0; i < DialogGraphPages.Num(); ++i)
	{
		UEdGraph* Graph = DialogGraphPages[i];
		if (Graph)
		{
			Graphs.Add(Graph);
			Graph->GetAllChildrenGraphs(Graphs);
		}
	}
	
}
#endif // WITH_EDITORONLY_DATA

#undef LOCTEXT_NAMESPACE