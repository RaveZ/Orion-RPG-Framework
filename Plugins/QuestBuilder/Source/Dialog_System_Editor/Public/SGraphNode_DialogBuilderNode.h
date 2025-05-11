// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"
#include "Widgets/SCompoundWidget.h"
#include "DialogBuilderEdNode.h"

class UDialogBuilderEdNode;
class DIALOG_SYSTEM_EDITOR_API SGraphNode_DialogBuilderNode : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SGraphNode_DialogBuilderNode)
	{}
	SLATE_END_ARGS()
	// SWidget interface 

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UDialogBuilderEdNode* InNode);
	virtual void UpdateGraphNode() override;
	virtual TArray<FOverlayWidgetInfo> GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const override;
	virtual void CreatePinWidgets() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	virtual bool IsNameReadOnly() const override;
	virtual FText GetDescription() const;
	virtual EVisibility GetDescriptionVisibility() const;
	virtual EVisibility GetTaskVisibility() const;
	virtual FText GetEventsText() const;
	virtual EVisibility GetEventsVisibility() const;
	virtual FText GetConditionsText() const;
	virtual EVisibility GetConditionsVisibility() const;

	
	/** Called when text is being committed to check for validity */
	bool OnVerifyNameTextChanged ( const FText& InText, FText& OutErrorMessage );

	/* Called when text is committed on the node */
	void OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo);

	FText GetPinTooltip(UEdGraphPin* GraphPinObj) const;

	virtual FSlateColor GetBorderBackgroundColor() const;
	virtual FSlateColor GetNodeHighlightColor() const;
	virtual FSlateColor GetBackgroundColor() const;

	virtual EVisibility GetDragOverMarkerVisibility() const;

	/** shows red marker when search failed*/
	EVisibility GetDebuggerSearchFailedMarkerVisibility() const;

	virtual const FSlateBrush* GetNameIcon() const;

protected:
	TSharedPtr<SHorizontalBox> OutputPinBox;
	TSharedPtr<SWidget> QuestNodeWidgetRef;

	/** The widget we use to display the index of the node */
	TSharedPtr<SWidget> IndexOverlay;

	EVisibility GetBlueprintIconVisibility() const;

	//Dialog Node Index

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
