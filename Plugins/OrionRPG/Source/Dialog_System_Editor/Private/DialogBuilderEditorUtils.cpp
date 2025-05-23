// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "DialogBuilderEditorUtils.h"
#include "DialogBuilderNode.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "DialogBuilderGraph.h"


#define LOCTEXT_NAMESPACE "DialogBuilderEditorUtils"

FDialogBuilderEditorUtils::FDialogBuilderEditorUtils()
{
}

FDialogBuilderEditorUtils::~FDialogBuilderEditorUtils()
{
}

FName FDialogBuilderEditorUtils::FindUniqueDialogName(const FString& InBaseName)
{
	// Start with the base name
	FString UniqueName = InBaseName;

	// Iterate through the dialog graph pages until we find a unique name
	for (int i = 0;; i++)
	{
		bool bIsUnique = true;

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

		TArray<FAssetData> DialogGraphDataArray;
		AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UDialogBuilderGraph::StaticClass()), DialogGraphDataArray);

		for (const FAssetData& AssetData : DialogGraphDataArray)
		{
			UDialogBuilderGraph* DialogGraph = Cast<UDialogBuilderGraph>(AssetData.GetAsset());
			if (DialogGraph)
			{
				for (auto& Dialog : DialogGraph->DialogList)
				{
					FString FullName = Dialog->ID.ToString();

					// If the current name matches a dialog graph page name, it's not unique
					if (FullName == UniqueName)
					{
						bIsUnique = false;
						break;
					}
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

FName FDialogBuilderEditorUtils::FindUniqueDialogGraphName(const FString& InBaseName)
{
	// Start with the base name
	FString UniqueName = InBaseName;

	// Iterate through the dialog graph pages until we find a unique name
	for (int i = 0;; i++)
	{
		bool bIsUnique = true;

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

		TArray<FAssetData> DialogGraphDataArray;
		AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UDialogBuilderGraph::StaticClass()), DialogGraphDataArray);

		for (const FAssetData& AssetData : DialogGraphDataArray)
		{
			UDialogBuilderGraph* DialogGraph = Cast<UDialogBuilderGraph>(AssetData.GetAsset());
			if (DialogGraph)
			{
				FString FullName = DialogGraph->ID.ToString();
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


UEdGraph* FDialogBuilderEditorUtils::CreateNewGraph(UObject* ParentScope, const FName& GraphName, TSubclassOf<class UEdGraph> GraphClass, TSubclassOf<class UEdGraphSchema> SchemaClass)
{
	UEdGraph* NewGraph = nullptr;

	
	// Construct a new graph with a default name
	NewGraph = NewObject<UEdGraph>(ParentScope, GraphClass, NAME_None, RF_Transactional);
	
	NewGraph->Schema = SchemaClass;
	NewGraph->GetSchema()->CreateDefaultNodesForGraph(*NewGraph);
	NewGraph->Rename(*(GraphName.ToString()), ParentScope, REN_DoNotDirty | REN_ForceNoResetLoaders);

	if (UDialogBuilderEdGraph* DialogGraph = Cast<UDialogBuilderEdGraph>(NewGraph))
	{
		DialogGraph->Dialog = NewObject<UDialog>(ParentScope, UDialog::StaticClass(), NAME_None, RF_Transactional);
		DialogGraph->Dialog->DialogGraph = DialogGraph->GetDialogBuilderGraph();
		DialogGraph->Dialog->ID = GraphName;
		DialogGraph->Dialog->DialogName = FText::FromName(DialogGraph->Dialog->ID);

		if (UDialogBuilderGraph* DialogBuilderGraph = Cast<UDialogBuilderGraph>(DialogGraph->GetDialogBuilderGraph()))
		{
			DialogBuilderGraph->DialogList.Add(DialogGraph->Dialog);
		}

	}

	
	return NewGraph;
}



void FDialogBuilderEditorUtils::RemoveNode(UDialogBuilderGraph* DialogBuilderGraph, UDialogBuilderEdNode* Node, bool bDontRecompile)
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

void FDialogBuilderEditorUtils::RemoveGraph(UDialogBuilderGraph* DialogBuilderGraph, UEdGraph* GraphToRemove)
{
	GraphToRemove->Modify();

	for (UObject* TestOuter = GraphToRemove->GetOuter(); TestOuter; TestOuter = TestOuter->GetOuter())
	{
		if (TestOuter == DialogBuilderGraph)
		{
			DialogBuilderGraph->DialogGraphPages.Remove(GraphToRemove);
			if (UDialogBuilderEdGraph* DialogGraphToRemove = Cast<UDialogBuilderEdGraph>(GraphToRemove))
			{
				DialogBuilderGraph->DialogList.Remove(DialogGraphToRemove->Dialog);
			}

			// Can't just call Remove, the object is wrapped in a struct
			for (int EditedDocIdx = 0; EditedDocIdx < DialogBuilderGraph->LastEditedDocuments.Num(); ++EditedDocIdx)
			{
				if (DialogBuilderGraph->LastEditedDocuments[EditedDocIdx].EditedObjectPath.ResolveObject() == GraphToRemove)
				{
					DialogBuilderGraph->LastEditedDocuments.RemoveAt(EditedDocIdx);
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
				FDialogBuilderEditorUtils::RemoveGraph(DialogBuilderGraph, SubGraph);
			}
		}
	}

	GraphToRemove->GetSchema()->HandleGraphBeingDeleted(*GraphToRemove);

	GraphToRemove->Rename(nullptr, DialogBuilderGraph ? DialogBuilderGraph->GetOuter() : nullptr, REN_DoNotDirty | REN_DontCreateRedirectors);
	GraphToRemove->ClearFlags(RF_Standalone | RF_Public);
	GraphToRemove->RemoveFromRoot();

	
}

void FDialogBuilderEditorUtils::AddDialogGraphPage(UDialogBuilderGraph* DialogBuilderGraph, UEdGraph* Graph)
{
#if WITH_EDITORONLY_DATA
	DialogBuilderGraph->DialogGraphPages.Add(Graph);
	Graph->MarkPackageDirty();
#endif	//#if WITH_EDITORONLY_DATA
}


UEdGraph* FDialogBuilderEditorUtils::FindDialogGraph(const UDialogBuilderGraph* DialogBuilderGraph)
{

	for (UEdGraph* CurrentGraph : DialogBuilderGraph->DialogGraphPages)
	{
		if (CurrentGraph->GetFName().ToString().Contains(TEXT("DialogGraph")))
		{
			UE_LOG(LogTemp, Warning, TEXT("Current graph name: %s"), *CurrentGraph->GetFName().ToString());
			return CurrentGraph;
		}
	}

	return nullptr;
}

TArray<UObject*> FDialogBuilderEditorUtils::GetSelectionForPropertyEditor(const TSet<UObject*>& InSelection)
{
	TArray<UObject*> Selection;

	for (UObject* SelectionEntry : InSelection)
	{
		UDialogBuilderEdNode* DialogEdNode = Cast<UDialogBuilderEdNode>(SelectionEntry);
		if (DialogEdNode)
		{
			Selection.Add(DialogEdNode->NodeInstance);
			continue;
		}
	}

	return Selection;
}


#undef LOCTEXT_NAMESPACE
