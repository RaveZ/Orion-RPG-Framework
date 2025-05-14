// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderGraph.h"
#include "Dialog.h"
#include "DialogComponent.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderEdge.h"

#define LOCTEXT_NAMESPACE "DialogBuilderGraph"

UDialogBuilderGraph::UDialogBuilderGraph()
{
}

UDialogBuilderGraph::~UDialogBuilderGraph()
{
	
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