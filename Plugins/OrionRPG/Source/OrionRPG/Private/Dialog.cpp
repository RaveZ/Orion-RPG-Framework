// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Dialog.h"
#include "DialogBuilderGraph.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderEdge.h"
#include "DialogBuilderNode_Speaker.h"
#include "DialogBuilderNode_Root.h"
#include "AssetRegistry/AssetRegistryModule.h"


#define LOCTEXT_NAMESPACE "Dialog"



UDialog::UDialog()
{
	bCanDialogBeAborted = false;
	bCanRetakeDialog = false;
	DialogState = EDialogState::E_Locked;
}

bool UDialog::Initialize(UDialogComponent* InitializingComp)
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		//We need a valid Dialog component to make a dialog for 
		if (!InitializingComp)
		{
			return false;
		}
	}
	return false;
}

void UDialog::Deinitialize()
{
	VisitedNodeIDs.Empty();
	CurrentNode = nullptr;
	
	for (auto& node : AllNodes)
	{
		node->Deinitialize();
	}
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

	VisitedNodeIDs.Empty();
	AllNodes.Empty();
	RootNodes.Empty();
	NodeMap.Empty();
}

void UDialog::EvaluateUniqueID()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> DialogGraphDataArray;
	AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UDialogBuilderGraph::StaticClass()), DialogGraphDataArray);

	TArray<UDialog*> Dialogs;
	TArray<FName> DialogIDs;

	for (const FAssetData& AssetData : DialogGraphDataArray)
	{
		UDialogBuilderGraph* DialogGraphAsset = Cast<UDialogBuilderGraph>(AssetData.GetAsset());
		if (DialogGraphAsset)
		{
			
		}
	}

	for (auto& Dialog : Dialogs)
	{
		if (Dialog != this)
		{
			DialogIDs.Add(Dialog->ID);
		}
	}

	int32 Suffix = 1;
	FName NewID = ID;

	if (!DialogIDs.Contains(NewID))
	{
		return;
	}

	// Check if the new ID already exists in the array
	while (DialogIDs.Contains(NewID))
	{
		// If it does, add a numeric suffix and try again
		NewID = FName(*FString::Printf(TEXT("%s%d"), *ID.ToString(), Suffix));
		Suffix++;
	}

	ID = NewID;

}

#if WITH_EDITOR
void UDialog::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty)
	{
		//If we changed the ID, make sure it doesn't conflict with any other IDs in the dialog
		if (PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UDialog, ID))
		{
			EvaluateUniqueID();
		}
	}
}
#endif

#undef LOCTEXT_NAMESPACE