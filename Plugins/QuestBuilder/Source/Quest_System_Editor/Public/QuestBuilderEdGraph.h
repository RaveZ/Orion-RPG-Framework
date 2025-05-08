// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "Quest.h"
#include "QuestBuilderEdGraph.generated.h"

class UQuestBuilderGraph;
class UQuestBuilderNode;
class UQuestBuilderEdge;
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

	virtual void RebuildQuestBuilderGraph(bool bForce = false);

	virtual void SortAllChildrenNodes(bool bForce = false);

	UQuestBuilderGraph* GetQuestSystemGraph() const;

	virtual bool Modify(bool bAlwaysMarkDirty = true) override;

	class SGraphEditor* SEditorGraph;

	/** Quest linked with this UEdGraph */
	UPROPERTY(VisibleAnywhere, Instanced, Category = "QuestBuilderGraph")
		TObjectPtr<UQuest> Quest;

	UPROPERTY(Transient)
		TMap< TObjectPtr<UQuestBuilderNode>, TObjectPtr<UQuestBuilderEdNode>> NodeMap;

	UPROPERTY(Transient)
		TMap< TObjectPtr<UQuestBuilderEdge>, TObjectPtr<UQuestBuilderEdNode_Edge>> EdgeMap;

	void Clear();

	void OnPackageMarkedDirty(UPackage* ModifiedPackage, bool bWasDirty);
protected:
	void SortNodes(UQuestBuilderNode* RootNode);
	double LastRebuildTime = 0.f;
	double LastSortTime = 0.f;

#if WITH_EDITOR
	virtual void PostEditUndo() override;
#endif
};
