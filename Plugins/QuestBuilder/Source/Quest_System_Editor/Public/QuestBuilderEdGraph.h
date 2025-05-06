// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "QuestSystem.h"
#include "QuestBuilderEdGraph.generated.h"

class UQuestSystemGraph;
class UQuestSystemGraphNode;
class UQuestSystemGraphEdge;
class UQuestBuilderEdNode;
class UQuestBuilderEdNode_Edge;
class UQuestBuilderEdNode_Edge;

UCLASS()
class QUEST_SYSTEM_EDITOR_API UQuestBuilderEdGraph : public UEdGraph
{
	GENERATED_BODY()
public:
	UQuestBuilderEdGraph();
	virtual ~UQuestBuilderEdGraph();

	virtual void RebuildQuestSystemGraph();

	UQuestSystemGraph* GetQuestSystemGraph() const;

	virtual bool Modify(bool bAlwaysMarkDirty = true) override;
	virtual void PostEditUndo() override;

	/** QuestSystem linked with this UEdGraph */
	UPROPERTY(VisibleAnywhere, Instanced, Category = "QuestSystemGraph")
		TObjectPtr<UQuestSystem> QuestSystem;

	UPROPERTY(Transient)
		TMap< TObjectPtr<UQuestSystemGraphNode>, TObjectPtr<UQuestBuilderEdNode>> NodeMap;

	UPROPERTY(Transient)
		TMap< TObjectPtr<UQuestSystemGraphEdge>, TObjectPtr<UQuestBuilderEdNode_Edge>> EdgeMap;


	void Clear();

protected:

	void SortNodes(UQuestSystemGraphNode* RootNode);
};
