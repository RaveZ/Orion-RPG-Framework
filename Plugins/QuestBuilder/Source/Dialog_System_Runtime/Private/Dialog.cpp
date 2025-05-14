// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Dialog.h"
#include "DialogBuilderGraph.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderEdge.h"
#include "DialogBuilderNode_Objective.h"
#include "DialogBuilderNode_Root.h"


#define LOCTEXT_NAMESPACE "Dialog"



UDialog::UDialog()
{
	bCanDialogBeAborted = false;
	bCanRetakeDialog = false;
	DialogState = EDialogState::E_Locked;
}

bool UDialog::Initialize(UDialogComponent* InitializingComp, UDialog* DialogTemplate)
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		//We need a valid narrative component to make a dialog for 
		if (!InitializingComp)
		{
			return false;
		}

		if (DialogTemplate)
		{
			
		}
	}
	return false;
}

int UDialog::GetLevelNum() const
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

void UDialog::GetNodesByLevel(int Level, TArray<UDialogBuilderNode*>& Nodes)
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

AController* UDialog::GetOwningController()
{
	if (!DialogGraph)
		return nullptr;

	return DialogGraph->OwningController;
}

APawn* UDialog::GetOwningPawn()
{
	if (!DialogGraph)
		return nullptr;

	return DialogGraph->OwningPawn;
}

UDialogComponent* UDialog::GetDialogComponent()
{
	if (!DialogGraph)
		return nullptr;

	return DialogGraph->DialogComponent;
}

UDialogBuilderGraph* UDialog::GetOwningDialogGraph() const
{
	if (!DialogGraph)
		return nullptr;

	return DialogGraph;
}

void UDialog::ClearGraph()
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

	AllNodes.Empty();
	RootNodes.Empty();
}

#if WITH_EDITOR
void UDialog::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (DialogGraph)
	{
	}
}
#endif

#undef LOCTEXT_NAMESPACE