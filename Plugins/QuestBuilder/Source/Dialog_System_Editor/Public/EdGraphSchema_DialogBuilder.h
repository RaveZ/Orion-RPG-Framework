// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "EdGraphSchema_DialogBuilder.generated.h"

/** Action to add a node to the graph */

class UDialogBuilderEdNode;
class UDialogBuilderEdNode_Edge;

struct FGraphNodeClassHelper;

UCLASS(MinimalAPI)
class UEdGraphSchema_DialogBuilder : public UEdGraphSchema
{
	GENERATED_BODY()
public:

	void GetBreakLinkToSubMenuActions(class UToolMenu* Menu, class UEdGraphPin* InGraphPin);

	//~ Begin EdGraphSchema Interface
	virtual void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;
	virtual EGraphType GetGraphType(const UEdGraph* TestEdGraph) const override;

	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;

	virtual void GetContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;

	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;

	virtual bool TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const override;
	virtual bool CreateAutomaticConversionNodeAndConnections(UEdGraphPin* A, UEdGraphPin* B) const override;

	virtual class FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const override;

	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;

	virtual void BreakNodeLinks(UEdGraphNode& TargetNode) const override;

	virtual void BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const override;

	virtual void BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const override;

	virtual UEdGraphPin* DropPinOnNode(UEdGraphNode* InTargetNode, const FName& InSourcePinName, const FEdGraphPinType& InSourcePinType, EEdGraphPinDirection InSourcePinDirection) const override;

	virtual bool SupportsDropPinOnNode(UEdGraphNode* InTargetNode, const FEdGraphPinType& InSourcePinType, EEdGraphPinDirection InSourcePinDirection, FText& OutErrorMessage) const override;

	virtual bool IsCacheVisualizationOutOfDate(int32 InVisualizationCacheID) const override;

	virtual int32 GetCurrentVisualizationCacheID() const override;

	virtual void ForceVisualizationCacheClear() const override;

protected:
	virtual FGraphNodeClassHelper& GetClassCache() const;
	static TSharedPtr<FAssetSchemaAction_DialogBuilder_NewNode> AddNewNodeAction(FGraphActionListBuilderBase& ContextMenuBuilder, const FText& Category, const FText& MenuDesc, const FText& Tooltip);
private:
	static int32 CurrentCacheRefreshID;
};

/** Action to add a node to the graph */
USTRUCT()
struct DIALOG_SYSTEM_EDITOR_API FAssetSchemaAction_DialogBuilder_NewNode : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

public:
	FAssetSchemaAction_DialogBuilder_NewNode() : NodeTemplate(nullptr) {}

	FAssetSchemaAction_DialogBuilder_NewNode(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), NodeTemplate(nullptr) {}

	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	TObjectPtr<UDialogBuilderEdNode> NodeTemplate;
};

USTRUCT()
struct DIALOG_SYSTEM_EDITOR_API FAssetSchemaAction_DialogBuilder_NewEdge : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

public:
	FAssetSchemaAction_DialogBuilder_NewEdge() : NodeTemplate(nullptr) {}

	FAssetSchemaAction_DialogBuilder_NewEdge(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), NodeTemplate(nullptr) {}

	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	TObjectPtr <UDialogBuilderEdNode_Edge> NodeTemplate;
};

/** Reference to a  event graph (only used in 'docked' palette) */
USTRUCT()
struct DIALOG_SYSTEM_EDITOR_API FAssetSchemaAction_DialogBuilderGraph : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY()

	// Simple type info
	static FName StaticGetTypeId() { static FName Type("FAssetSchemaAction_DialogBuilderGraph"); return Type; }
	virtual FName GetTypeId() const override { return StaticGetTypeId(); }

	/** Name of function or class */
	FName FuncName;


	/** The associated editor graph for this schema */
	UEdGraph* EdGraph;

	FAssetSchemaAction_DialogBuilderGraph()
		: FEdGraphSchemaAction()
	{}

	FAssetSchemaAction_DialogBuilderGraph(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping, const int32 InSectionID = 0)
		: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping, FText(), InSectionID)
		, EdGraph(nullptr)
	{}
};

