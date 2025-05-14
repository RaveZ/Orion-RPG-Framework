// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "Dialog.h"
#include "DialogBuilderEdGraph.generated.h"

class UDialogBuilderGraph;
class UDialogBuilderNode;
class UDialogBuilderEdge;
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

	virtual void RebuildDialogBuilderGraph(bool bForce = false);

	virtual void SortAllChildrenNodes(bool bForce = false);

	virtual void CacheNodeIDMap();

	virtual void AddToNodeIDMap(FName ID, UDialogBuilderNode* Node);

	UDialogBuilderGraph* GetDialogBuilderGraph() const;

	virtual bool Modify(bool bAlwaysMarkDirty = true) override;

	class SGraphEditor* SEditorGraph;

	/** Dialog linked with this UEdGraph */
	UPROPERTY(VisibleAnywhere, Instanced, Category = "DialogBuilderGraph")
		TObjectPtr<UDialog> Dialog;

		/*Map of NodID iwth its UDialogBuilderNode*/
	UPROPERTY(BlueprintReadOnly, Category = "DialogBuilderGraph")
		TMap<FName, TObjectPtr<UDialogBuilderNode>> NodeIDMap;

	UPROPERTY(Transient)
		TMap< TObjectPtr<UDialogBuilderNode>, TObjectPtr<UDialogBuilderEdNode>> NodeMap;

	UPROPERTY(Transient)
		TMap< TObjectPtr<UDialogBuilderEdge>, TObjectPtr<UDialogBuilderEdNode_Edge>> EdgeMap;

	void Clear();

	void OnPackageMarkedDirty(UPackage* ModifiedPackage, bool bWasDirty);
protected:
	void SortNodes(UDialogBuilderNode* RootNode);
	double LastRebuildTime = 0.f;
	double LastSortTime = 0.f;

#if WITH_EDITOR
	virtual void PostEditUndo() override;
#endif
};
