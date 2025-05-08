// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderGraph.h"
#include "Quest.h"
#include "QuestComponent.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderEdge.h"

#define LOCTEXT_NAMESPACE "QuestBuilderGraph"

UQuestBuilderGraph::UQuestBuilderGraph()
{
	bEdgeEnabled = false;
#if WITH_EDITORONLY_DATA

	bCanRenameNode = true;
#endif
}

UQuestBuilderGraph::~UQuestBuilderGraph()
{
	
}



#if WITH_EDITORONLY_DATA
void UQuestBuilderGraph::GetAllGraphs(TArray<UEdGraph*>& Graphs) const
{
	

	for (int32 i = 0; i < QuestGraphPages.Num(); ++i)
	{
		UEdGraph* Graph = QuestGraphPages[i];
		if (Graph)
		{
			Graphs.Add(Graph);
			Graph->GetAllChildrenGraphs(Graphs);
		}
	}
	
}
#endif // WITH_EDITORONLY_DATA

#undef LOCTEXT_NAMESPACE