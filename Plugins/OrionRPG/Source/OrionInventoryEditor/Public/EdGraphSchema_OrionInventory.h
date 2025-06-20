// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "EdGraphSchema_OrionInventory.generated.h"

namespace EInventorySubNode
{
	enum Type {
		Decorator,
		Event
	};
}

/** Action to add a node to the graph */

class UOrionInventoryEdNode;
class UOrionInventoryEdSubNode_Event;
class UOrionInventoryEdSubNode_Decorator;

struct FGraphNodeClassHelper;
struct FGraphNodeClassData;

UCLASS(MinimalAPI)
class UEdGraphSchema_OrionInventory : public UEdGraphSchema
{
	GENERATED_BODY()
public:
	UEdGraphSchema_OrionInventory();
	void GetBreakLinkToSubMenuActions(class UToolMenu* Menu, class UEdGraphPin* InGraphPin);

	//~ Begin EdGraphSchema Interface
	virtual void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;
	virtual EGraphType GetGraphType(const UEdGraph* TestEdGraph) const override;
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
	virtual void GetContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;
	virtual const FPinConnectionResponse CanMergeNodes(const UEdGraphNode* A, const UEdGraphNode* B) const override;
	virtual bool IsCacheVisualizationOutOfDate(int32 InVisualizationCacheID) const override;
	virtual int32 GetCurrentVisualizationCacheID() const override;
	virtual void ForceVisualizationCacheClear() const override;
	virtual TSharedPtr<FEdGraphSchemaAction> GetCreateCommentAction() const override;
	//end EdGraphSchema Interface

	virtual void GetGraphNodeContextActions(FGraphContextMenuBuilder& ContextMenuBuilder, int32 SubNodeFlags) const;
	virtual void GetSubNodeClasses(int32 SubNodeFlags, TArray<FGraphNodeClassData>& ClassData, UClass*& GraphNodeClass) const;


	TSubclassOf<UOrionInventoryEdSubNode_Decorator> DecoratorClass;
	TSubclassOf<UOrionInventoryEdSubNode_Event> EventClass;
protected:
	virtual FGraphNodeClassHelper& GetClassCache(int32 SubNodeFlags) const;
	static TSharedPtr<FAssetSchemaAction_Inventory_NewNode> AddNewNodeAction(FGraphActionListBuilderBase& ContextMenuBuilder, const FText& Category, const FText& MenuDesc, const FText& Tooltip);
	static TSharedPtr<FAssetSchemaAction_Inventory_NewSubNode> AddNewSubNodeAction(FGraphActionListBuilderBase& ContextMenuBuilder, const FText& Category, const FText& MenuDesc, const FText& Tooltip);

private:
	static int32 CurrentCacheRefreshID;
	
	/** creates add inventory item... submenu */
	void CreateAddInventoryItemSubMenu(class UToolMenu* Menu, UEdGraph* Graph) const;
};

/** Action to add a node to the graph */
USTRUCT()
struct ORIONINVENTORYEDITOR_API FAssetSchemaAction_Inventory_NewNode : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

public:
	FAssetSchemaAction_Inventory_NewNode() : NodeTemplate(nullptr) {}

	FAssetSchemaAction_Inventory_NewNode(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), NodeTemplate(nullptr) {}

	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	TObjectPtr<UOrionInventoryEdNode> NodeTemplate;
};

/** Reference to a  event graph (only used in 'docked' palette) */
USTRUCT()
struct ORIONINVENTORYEDITOR_API FAssetSchemaAction_InventoryGraph : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY()

	// Simple type info
	static FName StaticGetTypeId() { static FName Type("FAssetSchemaAction_InventoryGraph"); return Type; }
	virtual FName GetTypeId() const override { return StaticGetTypeId(); }

	/** Name of function or class */
	FName FuncName;


	/** The associated editor graph for this schema */
	UEdGraph* EdGraph;

	FAssetSchemaAction_InventoryGraph()
		: FEdGraphSchemaAction()
	{}

	FAssetSchemaAction_InventoryGraph(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping, const int32 InSectionID = 0)
		: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping, FText(), InSectionID)
		, EdGraph(nullptr)
	{}
};

/** Action to add a subnode to the selected node */
USTRUCT()
struct ORIONINVENTORYEDITOR_API FAssetSchemaAction_Inventory_NewSubNode : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

	/** Template of node we want to create */
	UPROPERTY()
	TObjectPtr<class UOrionInventoryEdNode> NodeTemplate;

	/** parent node */
	UPROPERTY()
	TObjectPtr<class UOrionInventoryEdNode> ParentNode;

	FAssetSchemaAction_Inventory_NewSubNode()
		: FEdGraphSchemaAction()
		, NodeTemplate(nullptr)
		, ParentNode(nullptr)
	{
	}

	FAssetSchemaAction_Inventory_NewSubNode(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)
		, NodeTemplate(nullptr)
		, ParentNode(nullptr)
	{
	}

	//~ Begin FEdGraphSchemaAction Interface
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2f& Location, bool bSelectNewNode = true) override;
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, TArray<UEdGraphPin*>& FromPins, const FVector2f& Location, bool bSelectNewNode = true) override;
#else if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 5)
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, TArray<UEdGraphPin*>& FromPins, const FVector2D Location, bool bSelectNewNode = true) override;
#endif
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End FEdGraphSchemaAction Interface
};


/** Action to add a comment to the graph */
USTRUCT()
struct ORIONINVENTORYEDITOR_API FInventorySchemaAction_AddComment : public FEdGraphSchemaAction
{
	GENERATED_BODY()

	FInventorySchemaAction_AddComment() : FEdGraphSchemaAction() {}
	FInventorySchemaAction_AddComment(FText InDescription, FText InToolTip)
		: FEdGraphSchemaAction(FText(), MoveTemp(InDescription), MoveTemp(InToolTip), 0)
	{
	}

	// FEdGraphSchemaAction interface
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2f& Location, bool bSelectNewNode = true) override final;
#else if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 5)
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override final;
#endif
	// End of FEdGraphSchemaAction interface
};