// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "Quest.h"
#include "DialogBuilderEdGraph.generated.h"

class UQuestBuilderGraph;
class UQuestBuilderNode;
class UQuestBuilderEdge;
class UDialogBuilderEdNode;
class UDialogBuilderEdNode_Edge;
class UDialogBuilderEdNode_Edge;

UCLASS()
class DIALOG_SYSTEM_EDITOR_API UDialogBuilderEdGraph : public UEdGraph
{
	GENERATED_BODY()
public:
	UDialogBuilderEdGraph();
	virtual ~UDialogBuilderEdGraph();

	virtual void RebuildQuestBuilderGraph(bool bForce = false);

	virtual void SortAllChildrenNodes(bool bForce = false);

	virtual void CacheNodeIDMap();

	virtual void AddToNodeIDMap(FName ID, UQuestBuilderNode* Node);

	UQuestBuilderGraph* GetQuestBuilderGraph() const;

	virtual bool Modify(bool bAlwaysMarkDirty = true) override;

	class SGraphEditor* SEditorGraph;

	/** Dialog linked with this UEdGraph */
	UPROPERTY(VisibleAnywhere, Instanced, Category = "QuestBuilderGraph")
		TObjectPtr<UQuest> Dialog;

		/*Map of NodID iwth its UQuestBuilderNode*/
	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderGraph")
		TMap<FName, TObjectPtr<UQuestBuilderNode>> NodeIDMap;

	UPROPERTY(Transient)
		TMap< TObjectPtr<UQuestBuilderNode>, TObjectPtr<UDialogBuilderEdNode>> NodeMap;

	UPROPERTY(Transient)
		TMap< TObjectPtr<UQuestBuilderEdge>, TObjectPtr<UDialogBuilderEdNode_Edge>> EdgeMap;

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
