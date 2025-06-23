// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"
#include "Widgets/SCompoundWidget.h"
#include "OrionInventoryEdNode.h"
#include "Editor/GraphEditor/Private/DragNode.h"

class FDragInventoryGraphNode : public FDragNode
{
public:
	DRAG_DROP_OPERATOR_TYPE(FDragInventoryGraphNode, FDragNode)

		static TSharedRef<FDragInventoryGraphNode> New(const TSharedRef<SGraphPanel>& InGraphPanel, const TSharedRef<SGraphNode>& InDraggedNode);
	static TSharedRef<FDragInventoryGraphNode> New(const TSharedRef<SGraphPanel>& InGraphPanel, const TArray< TSharedRef<SGraphNode> >& InDraggedNodes);

	UOrionInventoryEdNode* GetDropTargetNode() const;

	double StartTime;

protected:
	typedef FDragNode Super;
};

class UOrionInventoryEdNode;
class ORIONINVENTORYEDITOR_API SGraphNode_OrionInventoryNode : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SGraphNode_OrionInventoryNode)
	{}
	SLATE_END_ARGS()
	// SWidget interface 

	//~ Begin SGraphNode Interface
	void Construct(const FArguments& InArgs, UOrionInventoryEdNode* InNode);
	virtual void UpdateGraphNode() override;
	virtual TArray<FOverlayWidgetInfo> GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const override;
	virtual void OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnMouseMove(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent) override;
	virtual TSharedRef<SGraphNode> GetNodeUnderMouse(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void SetOwner(const TSharedRef<SGraphPanel>& OwnerPanel) override;
	virtual bool IsNameReadOnly() const override;
	virtual void CreatePinWidgets() override;
	virtual TSharedPtr<SToolTip> GetComplexTooltip() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
	virtual void MoveTo(const FVector2f& NewPosition, FNodeSet& NodeFilter, bool bMarkDirty = true) override;
#else if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 5)
	virtual void MoveTo(const FVector2D& NewPosition, FNodeSet& NodeFilter, bool bMarkDirty = true) override;
#endif
	//~ End SGraphNode Interface

	/** handle double click */
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;

	virtual FText GetDescription() const;
	virtual EVisibility GetDescriptionVisibility() const;
	virtual EVisibility GetInventoryItemVisibility() const;

	/**
	 * Ticks this widget.  Override in derived classes, but always call the parent implementation.
	 *
	 * @param  AllottedGeometry The space allotted for this widget
	 * @param  InCurrentTime  Current absolute real time
	 * @param  InDeltaTime  Real time passed since last tick
	 */
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	/** adds decorator widget inside current node */
	void AddInventoryItem(TSharedPtr<SGraphNode> DecoratorWidget, int32 Column, int32 Row);



	/** handle mouse down on the node */
	FReply OnMouseDown(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent);

	/** adds subnode widget inside current node */
	virtual void AddSubNode(TSharedPtr<SGraphNode> SubNodeWidget);

	/** gets decorator or service node if one is found under mouse cursor */
	TSharedPtr<SGraphNode> GetSubNodeUnderCursor(const FGeometry& WidgetGeometry, const FPointerEvent& MouseEvent);

	/** Called when text is being committed to check for validity */
	bool OnVerifyNameTextChanged ( const FText& InText, FText& OutErrorMessage );

	/* Called when text is committed on the node */
	void OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo);

	// Should we use low-detail node titles?
	virtual bool UseLowDetailNodeTitles() const;

	FText GetPinTooltip(UEdGraphPin* GraphPinObj) const;

	virtual FSlateColor GetBackgroundColor() const;
	virtual FSlateColor GetBorderBackgroundColor() const;

	virtual FText GetNodeIDText() const;
	
	virtual EVisibility GetNodeIDVisibility() const;
	virtual EVisibility GetTitleVisibility() const;
	virtual EVisibility GetParentNodeVisibility() const;
	virtual EVisibility GetSubNodeVisibility() const;
	virtual EVisibility GetDragOverMarkerVisibility() const;

	/** shows red marker when search failed*/
	EVisibility GetDebuggerSearchFailedMarkerVisibility() const;

	virtual const FSlateBrush* GetNameIcon() const;

	/** sets drag marker visible or collapsed on this node */
	void SetDragMarker(bool bEnabled);
protected:
	/** The node body widget, cached here so we can determine its size when we want ot position our overlays */
	TSharedPtr<SBorder> NodeBody;

	TArray< TSharedPtr<SGraphNode> > SubNodes;

	uint32 bDragMarkerVisible : 1;


	TArray< TSharedPtr<SGraphNode> > InventoryItemWidgets;
	TSharedPtr<SGridPanel> InventoryItemGridPanel;
	TSharedPtr<SHorizontalBox> OutputPinBox;
	TSharedPtr<SWidget> InventoryNodeWidgetRef;

	/** The widget we use to display the index of the node */
	TSharedPtr<SWidget> IndexOverlay;

	EVisibility GetBlueprintIconVisibility() const;

	//Inventory Node Index

	/** Get the visibility of the index overlay */
	EVisibility GetIndexVisibility() const;

	/** Get the text to display in the index overlay */
	FText GetIndexText() const;

	/** Get the tooltip for the index overlay */
	FText GetIndexTooltipText() const;

	/** Get the color to display for the index overlay. This changes on hover state of sibling nodes */
	FSlateColor GetIndexColor(bool bHovered) const;

	/** Handle hover state changing for the index widget - we use this to highlight sibling nodes */
	void OnIndexHoverStateChanged(bool bHovered);
};
