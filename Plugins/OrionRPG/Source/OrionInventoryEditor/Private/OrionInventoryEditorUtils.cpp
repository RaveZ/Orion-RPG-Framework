// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "OrionInventoryEditorUtils.h"
#include "DialogBuilderNode.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "DialogBuilderGraph.h"


#define LOCTEXT_NAMESPACE "OrionInventoryEditorUtils"

FOrionInventoryEditorUtils::FOrionInventoryEditorUtils()
{
}

FOrionInventoryEditorUtils::~FOrionInventoryEditorUtils()
{
}

FName FOrionInventoryEditorUtils::FindUniqueInventoryName(const FString& InBaseName)
{
	// Start with the base name
	FString UniqueName = InBaseName;

	// Iterate through the inventory graph pages until we find a unique name
	for (int i = 0;; i++)
	{
		bool bIsUnique = true;

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

		TArray<FAssetData> InventoryGraphDataArray;
		AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UDialogBuilderGraph::StaticClass()), InventoryGraphDataArray);

		for (const FAssetData& AssetData : InventoryGraphDataArray)
		{
			UDialogBuilderGraph* InventoryGraph = Cast<UDialogBuilderGraph>(AssetData.GetAsset());
			if (InventoryGraph)
			{
				
			}
		}

		// If the current name is unique, return it
		if (bIsUnique)
		{
			return FName(*UniqueName);
		}

		// If the current name is not unique, append a number to it and try again
		UniqueName = FString::Printf(TEXT("%s%d"), *InBaseName, i + 1);
	}
}

FName FOrionInventoryEditorUtils::FindUniqueInventoryGraphName(const FString& InBaseName)
{
	// Start with the base name
	FString UniqueName = InBaseName;

	// Iterate through the inventory graph pages until we find a unique name
	for (int i = 0;; i++)
	{
		bool bIsUnique = true;

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

		TArray<FAssetData> InventoryGraphDataArray;
		AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UDialogBuilderGraph::StaticClass()), InventoryGraphDataArray);

		for (const FAssetData& AssetData : InventoryGraphDataArray)
		{
			UDialogBuilderGraph* InventoryGraph = Cast<UDialogBuilderGraph>(AssetData.GetAsset());
			if (InventoryGraph)
			{
				FString FullName = InventoryGraph->ID.ToString();
				if (FullName == UniqueName)
				{
					bIsUnique = false;
					break;
				}
			}
		}

		// If the current name is unique, return it
		if (bIsUnique)
		{
			return FName(*UniqueName);
		}

		// If the current name is not unique, append a number to it and try again
		UniqueName = FString::Printf(TEXT("%s%d"), *InBaseName, i + 1);
	}
}


UEdGraph* FOrionInventoryEditorUtils::CreateNewGraph(UObject* ParentScope, const FName& GraphName, TSubclassOf<class UEdGraph> GraphClass, TSubclassOf<class UEdGraphSchema> SchemaClass)
{
	UEdGraph* NewGraph = nullptr;

	
	// Construct a new graph with a default name
	NewGraph = NewObject<UEdGraph>(ParentScope, GraphClass, NAME_None, RF_Transactional);
	
	NewGraph->Schema = SchemaClass;
	NewGraph->GetSchema()->CreateDefaultNodesForGraph(*NewGraph);
	NewGraph->Rename(*(GraphName.ToString()), ParentScope, REN_DoNotDirty | REN_ForceNoResetLoaders);

	if (UOrionInventoryEdGraph* InventoryGraph = Cast<UOrionInventoryEdGraph>(NewGraph))
	{
		InventoryGraph->OwningInventory = CastChecked<UDialogBuilderGraph>(ParentScope);
	}

	
	return NewGraph;
}



void FOrionInventoryEditorUtils::RemoveNode(UDialogBuilderGraph* OrionInventoryGraph, UOrionInventoryEdNode* Node, bool bDontRecompile)
{
	check(Node);

	const UEdGraphSchema* Schema = nullptr;

	// Ensure we mark parent graph modified
	if (UEdGraph* GraphObj = Node->GetGraph())
	{
		GraphObj->Modify();
		Schema = GraphObj->GetSchema();
	}

	Node->Modify();

	// Timelines will be removed from the blueprint if the node is a UK2Node_Timeline
	if (Schema)
	{
		Schema->BreakNodeLinks(*Node);
	}

	Node->DestroyNode();

	/*if (!bDontRecompile && (Blueprint != nullptr))
	{
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	}*/
}

void FOrionInventoryEditorUtils::RemoveGraph(UDialogBuilderGraph* OrionInventoryGraph, UEdGraph* GraphToRemove)
{
	GraphToRemove->Modify();

	for (UObject* TestOuter = GraphToRemove->GetOuter(); TestOuter; TestOuter = TestOuter->GetOuter())
	{
		if (TestOuter == OrionInventoryGraph)
		{
			OrionInventoryGraph->DialogGraphPages.Remove(GraphToRemove);

			// Can't just call Remove, the object is wrapped in a struct
			for (int EditedDocIdx = 0; EditedDocIdx < OrionInventoryGraph->LastEditedDocuments.Num(); ++EditedDocIdx)
			{
				if (OrionInventoryGraph->LastEditedDocuments[EditedDocIdx].EditedObjectPath.ResolveObject() == GraphToRemove)
				{
					OrionInventoryGraph->LastEditedDocuments.RemoveAt(EditedDocIdx);
					break;
				}
			}
		}
		else if (UEdGraph* OuterGraph = Cast<UEdGraph>(TestOuter))
		{
			// remove ourselves
			OuterGraph->Modify();
			OuterGraph->SubGraphs.Remove(GraphToRemove);
		}
		else if (!(Cast<UEdGraphNode>(TestOuter) && Cast<UEdGraphNode>(TestOuter)->GetSubGraphs().Num() > 0))
		{
			break;
		}
	}

	

	// Handle subgraphs held in graph
	TArray<UEdGraphNode*> AllNodes;
	GraphToRemove->GetNodesOfClass<UEdGraphNode>(AllNodes);

	for (UEdGraphNode* GraphNode : AllNodes)
	{
		for (UEdGraph* SubGraph : GraphNode->GetSubGraphs())
		{
			if (SubGraph && SubGraph->GetOuter()->IsA(UEdGraphNode::StaticClass()))
			{
				FOrionInventoryEditorUtils::RemoveGraph(OrionInventoryGraph, SubGraph);
			}
		}
	}

	GraphToRemove->GetSchema()->HandleGraphBeingDeleted(*GraphToRemove);

	GraphToRemove->Rename(nullptr, OrionInventoryGraph ? OrionInventoryGraph->GetOuter() : nullptr, REN_DoNotDirty | REN_DontCreateRedirectors);
	GraphToRemove->ClearFlags(RF_Standalone | RF_Public);
	GraphToRemove->RemoveFromRoot();

	
}

void FOrionInventoryEditorUtils::AddInventoryGraphPage(UDialogBuilderGraph* OrionInventoryGraph, UEdGraph* Graph)
{
#if WITH_EDITORONLY_DATA
	OrionInventoryGraph->DialogGraphPages.Add(Graph);
	Graph->MarkPackageDirty();
#endif	//#if WITH_EDITORONLY_DATA
}


UEdGraph* FOrionInventoryEditorUtils::FindInventoryGraph(const UDialogBuilderGraph* OrionInventoryGraph)
{

	for (UEdGraph* CurrentGraph : OrionInventoryGraph->DialogGraphPages)
	{
		if (CurrentGraph->GetFName().ToString().Contains(TEXT("InventoryGraph")))
		{
			UE_LOG(LogTemp, Warning, TEXT("Current graph name: %s"), *CurrentGraph->GetFName().ToString());
			return CurrentGraph;
		}
	}

	return nullptr;
}

TArray<UObject*> FOrionInventoryEditorUtils::GetSelectionForPropertyEditor(const TSet<UObject*>& InSelection)
{
	TArray<UObject*> Selection;

	for (UObject* SelectionEntry : InSelection)
	{
		UOrionInventoryEdNode* InventoryEdNode = Cast<UOrionInventoryEdNode>(SelectionEntry);
		if (InventoryEdNode)
		{
			Selection.Add(InventoryEdNode->NodeInstance);
			continue;
		}
		Selection.Add(SelectionEntry);
	}

	return Selection;
}


#undef LOCTEXT_NAMESPACE
