// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "OrionInventoryEdGraph.generated.h"

class UOrionInventoryGraph;
class UOrionInventoryNode;
class UOrionInventoryEdNode;

UCLASS()
class ORIONINVENTORYEDITOR_API UOrionInventoryEdGraph : public UEdGraph
{
	GENERATED_BODY()
public:
	UOrionInventoryEdGraph();
	virtual ~UOrionInventoryEdGraph();

	virtual void UpdateAsset(bool bForce = false);
	virtual void SortAllChildrenNodes(bool bForce = false);
	virtual void UpdateAllSubnodeDependencies();
	virtual void RebuildChildOrder(UEdGraphNode* ParentNode);
	virtual void OnSubNodeDropped();

	bool UpdateUnknownNodeClasses();
	void UpdateDeprecatedClasses();
	void UpdateClassData();
	void RemoveUnknownSubNodes();

	bool IsLocked() const;
	void LockUpdates();
	void UnlockUpdates();

	UOrionInventoryGraph* GetOrionInventoryGraph() const;

	virtual bool Modify(bool bAlwaysMarkDirty = true) override;

	class SGraphEditor* SEditorGraph;
	
	/** Inventory linked with this UEdGraph */
	UPROPERTY(VisibleAnywhere, Instanced, Category = "OrionInventoryGraph")
	TObjectPtr<UOrionInventoryGraph> OwningInventory;

	UPROPERTY(Transient)
		TMap< TObjectPtr<UOrionInventoryNode>, TObjectPtr<UOrionInventoryEdNode>> NodeMap;

	void Clear();

	void OnPackageMarkedDirty(UPackage* ModifiedPackage, bool bWasDirty);
protected:
	/** if set, graph modifications won't cause updates in internal tree structure
	 *  flag allows freezing update during heavy changes like pasting new nodes
	 */
	uint32 bLockUpdates : 1;

	double LastRebuildTime = 0.f;
	double LastSortTime = 0.f;

#if WITH_EDITOR
	virtual void PostEditUndo() override;
#endif
protected:
	virtual void CollectAllNodeInstances(TSet<UObject*>& NodeInstances);
	void UpdateVersion_UnifiedSubNodes();
};
