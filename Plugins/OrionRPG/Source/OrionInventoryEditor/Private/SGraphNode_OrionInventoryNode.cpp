// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "SGraphNode_OrionInventoryNode.h"
#include "SlateOptMacros.h"
#include "Types/SlateStructs.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SToolTip.h"
#include "SGraphPanel.h"
#include "NodeFactory.h"
#include "Components/VerticalBox.h"
#include "SGraphPreviewer.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "SGraphPin.h"
#include "Colors_OrionInventory.h"
#include "GraphEditorSettings.h"
#include "SCommentBubble.h"
#include "SLevelOfDetailBranchNode.h"
#include "DialogBuilderNode.h"
#include "OrionInventoryEdNode_Root.h"
#include "OrionInventoryEdNode_Speaker.h"
#include "DialogBuilderNode_Speaker.h"
#include "DialogBuilderSetting.h"	
#include "Fonts/SlateFontInfo.h"
#include "Decorator/OrionDecorator.h"
#include "Event/OrionEvent.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "OrionInventoryEdSubNode_Decorator.h"
#include "OrionInventoryEdSubNode_Event.h"
#include "IDocumentation.h"

#define LOCTEXT_NAMESPACE "EdNode_OrionInventory"

#define ALWAYS_SHOW_QUEST_EXECUTION_INDEX 1

bool ShouldShowExecutionIndex()
{
#ifdef ALWAYS_SHOW_QUEST_EXECUTION_INDEX
	return true;
#else
	return GEditor && (GEditor->bIsSimulatingInEditor || GEditor->PlayWorld != NULL);
#endif // ALWAYS_SHOW_BT_EXECUTION_INDEX
}

//////////////////////////////////////////////////////////////////////////
class SInventorySystemPin : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SInventorySystemPin) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InPin)
	{
		this->SetCursor(EMouseCursor::Default);

		bShowLabel = true;

		GraphPinObj = InPin;
		check(GraphPinObj != nullptr);

		const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
		check(Schema);

		SBorder::Construct(SBorder::FArguments()
			.BorderImage(this, &SInventorySystemPin::GetPinBorder)
			.BorderBackgroundColor(this, &SInventorySystemPin::GetPinColor)
			.OnMouseButtonDown(this, &SInventorySystemPin::OnPinMouseDown)
			.Cursor(this, &SInventorySystemPin::GetPinCursor)
			.Padding(FMargin(5.0f))
		);
	}

protected:
	virtual FSlateColor GetPinColor() const override
	{
		return bIsDiffHighlighted ? OrionInventoryColors::Pin::Diff :
			IsHovered() ? OrionInventoryColors::Pin::Hover : OrionInventoryColors::Pin::Default;
	}

	virtual TSharedRef<SWidget>	GetDefaultValueWidget() override
	{
		return SNew(STextBlock);
	}

	const FSlateBrush* GetPinBorder() const
	{
		return FAppStyle::GetBrush(TEXT("Graph.StateNode.Body"));
	}

};


/** Widget for overlaying an execution-order index onto a node */
class SInventoryNodeIndex : public SCompoundWidget
{
public:
	/** Delegate event fired when the hover state of this widget changes */
	DECLARE_DELEGATE_OneParam(FOnHoverStateChanged, bool /* bHovered */);

	/** Delegate used to receive the color of the node, depending on hover state and state of other siblings */
	DECLARE_DELEGATE_RetVal_OneParam(FSlateColor, FOnGetIndexColor, bool /* bHovered */);

	SLATE_BEGIN_ARGS(SInventoryNodeIndex) {}
		SLATE_ATTRIBUTE(FText, Text)
		SLATE_EVENT(FOnHoverStateChanged, OnHoverStateChanged)
		SLATE_EVENT(FOnGetIndexColor, OnGetIndexColor)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		OnHoverStateChangedEvent = InArgs._OnHoverStateChanged;
		OnGetIndexColorEvent = InArgs._OnGetIndexColor;

		const FSlateBrush* IndexBrush = FAppStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Index"));

		ChildSlot
			[
				SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						// Add a dummy box here to make sure the widget doesnt get smaller than the brush
						SNew(SBox)
							.WidthOverride(IndexBrush->ImageSize.X)
							.HeightOverride(IndexBrush->ImageSize.Y)
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SBorder)
							.BorderImage(IndexBrush)
							.BorderBackgroundColor(this, &SInventoryNodeIndex::GetColor)
							.Padding(FMargin(4.0f, 0.0f, 4.0f, 1.0f))
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							[
								SNew(STextBlock)
									.Text(InArgs._Text)
									.Font(FAppStyle::GetFontStyle("BTEditor.Graph.BTNode.IndexText"))
							]
					]
			];
	}

	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		OnHoverStateChangedEvent.ExecuteIfBound(true);
		SCompoundWidget::OnMouseEnter(MyGeometry, MouseEvent);
	}

	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override
	{
		OnHoverStateChangedEvent.ExecuteIfBound(false);
		SCompoundWidget::OnMouseLeave(MouseEvent);
	}

	/** Get the color we use to display the rounded border */
	FSlateColor GetColor() const
	{
		if (OnGetIndexColorEvent.IsBound())
		{
			return OnGetIndexColorEvent.Execute(IsHovered());
		}

		return FSlateColor::UseForeground();
	}

private:
	/** Delegate event fired when the hover state of this widget changes */
	FOnHoverStateChanged OnHoverStateChangedEvent;

	/** Delegate used to receive the color of the node, depending on hover state and state of other siblings */
	FOnGetIndexColor OnGetIndexColorEvent;
};


void SGraphNode_OrionInventoryNode::Construct(const FArguments& InArgs, UOrionInventoryEdNode* InNode)
{
	SetCursor(EMouseCursor::CardinalCross);
	GraphNode = InNode;
	UpdateGraphNode();
	InNode->SGraphNode = this;
	bDragMarkerVisible = false;
}


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SGraphNode_OrionInventoryNode::UpdateGraphNode()
{
	bDragMarkerVisible = false;
	InputPins.Empty();
	OutputPins.Empty();

	if (DecoratorsBox.IsValid())
	{
		DecoratorsBox->ClearChildren();
	}
	else
	{
		SAssignNew(DecoratorsBox, SVerticalBox);
	}

	if (EventsBox.IsValid())
	{
		EventsBox->ClearChildren();
	}
	else
	{
		SAssignNew(EventsBox, SVerticalBox);
	}

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset(); 
	DecoratorWidgets.Reset();
	EventsWidgets.Reset();
	SubNodes.Reset();
	OutputPinBox.Reset();

	UOrionInventoryEdNode* InventoryEdNode = Cast<UOrionInventoryEdNode>(GraphNode);
	UDialogBuilderNode* InventoryNode = InventoryEdNode ? Cast<UDialogBuilderNode>(InventoryEdNode->NodeInstance) : nullptr;

	if (InventoryEdNode)
	{
		for (int32 i = 0; i < InventoryEdNode->Decorators.Num(); i++)
		{
			if (InventoryEdNode->Decorators[i])
			{
				TSharedPtr<SGraphNode> NewNode = FNodeFactory::CreateNodeWidget(InventoryEdNode->Decorators[i]);
				if (OwnerGraphPanelPtr.IsValid())
				{
					NewNode->SetOwner(OwnerGraphPanelPtr.Pin().ToSharedRef());
					OwnerGraphPanelPtr.Pin()->AttachGraphEvents(NewNode);
				}
				AddDecorator(NewNode);
				NewNode->UpdateGraphNode();
			}
		}
		for (int32 i = 0; i < InventoryEdNode->Events.Num(); i++)
		{
			if (InventoryEdNode->Events[i])
			{
				TSharedPtr<SGraphNode> NewNode = FNodeFactory::CreateNodeWidget(InventoryEdNode->Events[i]);
				if (OwnerGraphPanelPtr.IsValid())
				{
					NewNode->SetOwner(OwnerGraphPanelPtr.Pin().ToSharedRef());
					OwnerGraphPanelPtr.Pin()->AttachGraphEvents(NewNode);
				}
				AddEvent(NewNode);
				NewNode->UpdateGraphNode();
			}
		}
	}

	TSharedPtr<SErrorText> ErrorText;
	TSharedPtr<STextBlock> DescriptionText;
	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);
	TWeakPtr<SNodeTitle> WeakNodeTitle = NodeTitle;

	auto GetNodeTitlePlaceholderWidth = [WeakNodeTitle]() -> FOptionalSize
		{
			TSharedPtr<SNodeTitle> NodeTitlePin = WeakNodeTitle.Pin();
			const float DesiredWidth = (NodeTitlePin.IsValid()) ? NodeTitlePin->GetTitleSize().X : 0.0f;
			return FMath::Max(75.0f, DesiredWidth);
		};
	auto GetNodeTitlePlaceholderHeight = [WeakNodeTitle]() -> FOptionalSize
		{
			TSharedPtr<SNodeTitle> NodeTitlePin = WeakNodeTitle.Pin();
			const float DesiredHeight = (NodeTitlePin.IsValid()) ? NodeTitlePin->GetTitleSize().Y : 0.0f;
			return FMath::Max(22.0f, DesiredHeight);
		};

	const FMargin NodePadding = (Cast<UOrionInventoryEdSubNode_Decorator>(GraphNode) || Cast<UOrionInventoryEdSubNode_Event>(GraphNode))
		? FMargin(1.f)
		: FMargin(8.0f);

	const float NodeMaxWidth = InventoryEdNode && InventoryEdNode->IsSubNode()
		? 325.f
		: 350.f;

	const float NodeMinWidth = InventoryEdNode && InventoryEdNode->IsSubNode()
		? 200.f
		: 125.f;

	const FMargin PinPadding = (Cast<UOrionInventoryEdSubNode_Decorator>(GraphNode) || Cast<UOrionInventoryEdSubNode_Event>(GraphNode))
		? FMargin(0.f)
		: FMargin(2.f, 3.f, 2.f, 3.f);

	UWorld* World = GEditor->GetEditorWorldContext().World();

	IndexOverlay = SNew(SInventoryNodeIndex)
		.ToolTipText(this, &SGraphNode_OrionInventoryNode::GetIndexTooltipText)
		.Visibility(this, &SGraphNode_OrionInventoryNode::GetIndexVisibility)
		.Text(this, &SGraphNode_OrionInventoryNode::GetIndexText)
		.OnHoverStateChanged(this, &SGraphNode_OrionInventoryNode::OnIndexHoverStateChanged)
		.OnGetIndexColor(this, &SGraphNode_OrionInventoryNode::GetIndexColor);
	

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
			.Padding(0.0f)
			.BorderBackgroundColor(this, &SGraphNode_OrionInventoryNode::GetBorderBackgroundColor)
			.OnMouseButtonDown(this, &SGraphNode_OrionInventoryNode::OnMouseDown)
			[
				SNew(SOverlay)
				// Pins and node details
				+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SHorizontalBox)

							// INPUT PIN AREA
							+ SHorizontalBox::Slot()
							.Padding(PinPadding)
							.AutoWidth()
							[
								SNew(SBox)
									.MinDesiredWidth(NodePadding.Left)
									[
										SAssignNew(LeftNodeBox, SVerticalBox)
									]
							]

							// STATE NAME AREA
							+ SHorizontalBox::Slot()
							.Padding(FMargin(NodePadding))
							[
								SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.AutoHeight()
									[
										SAssignNew(NodeBody, SBorder)
										.BorderImage(FAppStyle::GetBrush("BTEditor.Graph.BTNode.Body"))
										.BorderBackgroundColor(this, &SGraphNode_OrionInventoryNode::GetBackgroundColor)
										.HAlign(HAlign_Fill)
										.VAlign(VAlign_Center)
										.Visibility(EVisibility::SelfHitTestInvisible)
										[
											SNew(SOverlay)
											+ SOverlay::Slot()
											.HAlign(HAlign_Fill)
											.VAlign(VAlign_Fill)
											[
												SNew(SVerticalBox)
												+ SVerticalBox::Slot()
												.HAlign(HAlign_Fill)
												.VAlign(VAlign_Center)
												.AutoHeight()
												[
													SNew(SHorizontalBox)
													.Visibility(this, &SGraphNode_OrionInventoryNode::GetTitleVisibility)
													+ SHorizontalBox::Slot()
													.AutoWidth()
													[
														// POPUP ERROR MESSAGE
														SAssignNew(ErrorText, SErrorText)
															.BackgroundColor(this, &SGraphNode_OrionInventoryNode::GetErrorColor)
															.ToolTipText(this, &SGraphNode_OrionInventoryNode::GetErrorMsgToolTip)
													]
													+ SHorizontalBox::Slot()
													.AutoWidth()
													[
															SNew(SHorizontalBox)
															+ SHorizontalBox::Slot()
															.AutoWidth()
															.VAlign(VAlign_Top)
															.Padding(FMargin(0.0f, 5.0f, 0.0f, 0.0f))
															[
																SNew(SImage)
																	.Image(this, &SGraphNode_OrionInventoryNode::GetNameIcon)
																	//.Visibility(this, &SGraphNode_OrionInventoryNode::GetParentNodeVisibility)
															]
																//if parent node
															+ SHorizontalBox::Slot()
															.MinWidth(NodeMinWidth)
															.MaxWidth(NodeMaxWidth)
															.Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
															[
																SNew(SVerticalBox)
																.Visibility(this, &SGraphNode_OrionInventoryNode::GetParentNodeVisibility)
																+ SVerticalBox::Slot()
																.AutoHeight()
																[
																	SAssignNew(InlineEditableText, SInlineEditableTextBlock)
																		.Style(FAppStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText")
																		.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
																		.OnVerifyTextChanged(this, &SGraphNode_OrionInventoryNode::OnVerifyNameTextChanged)
																		.OnTextCommitted(this, &SGraphNode_OrionInventoryNode::OnNameTextCommited)
																		.IsReadOnly(this, &SGraphNode_OrionInventoryNode::IsNameReadOnly)
																		.IsSelected(this, &SGraphNode_OrionInventoryNode::IsSelectedExclusively)
																		.AutoWrapMultilineEditText(true)
																					
																]
																+ SVerticalBox::Slot()
																.AutoHeight()
																[
																	NodeTitle.ToSharedRef()
																]
																+ SVerticalBox::Slot()
																.AutoHeight()
																.Padding(FMargin(2.0f, 3.0f, 2.0f, 4.0f))
																[
																	// NodeTag
																	SAssignNew(DescriptionText, STextBlock)
																		.Visibility(this, &SGraphNode_OrionInventoryNode::GetNodeIDVisibility)
																		.Text(this, &SGraphNode_OrionInventoryNode::GetNodeIDText)
																		.TextStyle(FAppStyle::Get(), TEXT("RichTextBlock.Bold"))
																		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
																]
																+ SVerticalBox::Slot()
																.AutoHeight()
																.Padding(FMargin(2.0f, 0.0f, 2.0f, 4.0f))
																[
																	// DESCRIPTION MESSAGE
																	SAssignNew(DescriptionText, STextBlock)
																		.Visibility(this, &SGraphNode_OrionInventoryNode::GetDescriptionVisibility)
																		.Text(this, &SGraphNode_OrionInventoryNode::GetDescription)
																		.TextStyle(FAppStyle::Get(), TEXT("RichTextBlock.Italic"))
																		.AutoWrapText(true)
																		.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
																]
															]
																//if subnode
															+ SHorizontalBox::Slot()
															.MinWidth(NodeMinWidth)
															.MaxWidth(NodeMaxWidth)
															.Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
															[
																SNew(SVerticalBox)
																.Visibility(this, &SGraphNode_OrionInventoryNode::GetSubNodeVisibility)
																+ SVerticalBox::Slot()
																.AutoHeight()
																[
																	SNew(STextBlock)
																		.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
																		.TextStyle(FAppStyle::Get(), TEXT("PhysicsAssetEditor.Tools.Font"))
																		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9.5))
																		.Clipping(EWidgetClipping::ClipToBounds)
																]
																+ SVerticalBox::Slot()
																.AutoHeight()
																[
																	NodeTitle.ToSharedRef()
																]
																+ SVerticalBox::Slot()
																
																.AutoHeight()
																	.Padding(FMargin(1.0f, 0.0f, 1.0f, 1.0f))
																[
																			// DESCRIPTION MESSAGE
																			SAssignNew(DescriptionText, STextBlock)
																				.Visibility(this, &SGraphNode_OrionInventoryNode::GetDescriptionVisibility)
																				.Text(this, &SGraphNode_OrionInventoryNode::GetDescription)
																				.TextStyle(FAppStyle::Get(), TEXT("RichTextBlock.Italic"))
																				.AutoWrapText(true)
																				.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
																]
															]
														
													]
												]	
														
											]
										]
									]
									+ SVerticalBox::Slot()
									.AutoHeight()
									[
										SAssignNew(NodeBody, SBorder)
											.BorderImage(FAppStyle::GetBrush("BTEditor.Graph.BTNode.Body"))
											.BorderBackgroundColor(OrionInventoryColors::NodeBorder::SubNodeBorder)
											.HAlign(HAlign_Fill)
											.VAlign(VAlign_Center)
											.Visibility(this, &SGraphNode_OrionInventoryNode::GetEventsVisibility)
											[
												SNew(SOverlay)
													+ SOverlay::Slot()
													.HAlign(HAlign_Fill)
													.VAlign(VAlign_Fill)
													[
														SNew(SVerticalBox)
															+ SVerticalBox::Slot()
															.HAlign(HAlign_Fill)
															.VAlign(VAlign_Center)
															.AutoHeight()
															[
																SNew(SHorizontalBox)
																	/*+ SHorizontalBox::Slot()
																	.AutoWidth()
																	.VAlign(VAlign_Center)
																	[
																		SNew(SImage)
																			.Image(FAppStyle::GetBrush(TEXT("GraphEditor.CustomEvent_16x")))
																	]*/
																	+ SHorizontalBox::Slot()
																	.Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
																	[
																		SNew(STextBlock)
																			.Text(LOCTEXT("EventLabel", "Events"))
																			.TextStyle(FAppStyle::Get(), TEXT("PhysicsAssetEditor.Tools.Font"))
																			.Clipping(EWidgetClipping::Inherit)
																	]
															]
															+ SVerticalBox::Slot()
															.Padding(FMargin(8.0f, 0, 0, 0))
															.AutoHeight()
															[
																EventsBox.ToSharedRef()
															]
													]
											]
									]
									+ SVerticalBox::Slot()
									.AutoHeight()
									[
										SAssignNew(NodeBody, SBorder)
											.BorderImage(FAppStyle::GetBrush("BTEditor.Graph.BTNode.Body"))
											.BorderBackgroundColor(OrionInventoryColors::NodeBorder::SubNodeBorder)
											.HAlign(HAlign_Fill)
											.VAlign(VAlign_Center)
											.Visibility(this, &SGraphNode_OrionInventoryNode::GetDecoratorVisibility)
											[
												SNew(SOverlay)
													+ SOverlay::Slot()
													.HAlign(HAlign_Fill)
													.VAlign(VAlign_Fill)
													[
														SNew(SVerticalBox)
															+ SVerticalBox::Slot()
															.HAlign(HAlign_Fill)
															.VAlign(VAlign_Center)
															.AutoHeight()
															[
																SNew(SHorizontalBox)
																	/*+ SHorizontalBox::Slot()
																	.AutoWidth()
																	.VAlign(VAlign_Center)
																	[
																		SNew(SImage)
																			.Image(FAppStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Decorator.Conditional.Icon")))
																	]*/
																	+ SHorizontalBox::Slot()
																	.Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
																	[
																		SNew(STextBlock)
																			.Text(LOCTEXT("DecoratorLabel", "Decorators"))
																			.TextStyle(FAppStyle::Get(), TEXT("PhysicsAssetEditor.Tools.Font"))
																			.Clipping(EWidgetClipping::Inherit)
																	]
															]
															+ SVerticalBox::Slot()
															.Padding(FMargin(8.0f, 0, 0, 0))
															.AutoHeight()
															[
																DecoratorsBox.ToSharedRef()
															]
													]
											]
									]

							]

							// OUTPUT PIN AREA
							+ SHorizontalBox::Slot()
							.Padding(PinPadding)
							.AutoWidth()
							[
								SNew(SBox)
									.MinDesiredWidth(NodePadding.Right)
									[
										SAssignNew(RightNodeBox, SVerticalBox)
									]
							]
					]
					// Drag marker overlay
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Top)
					[
						SNew(SBorder)
							.BorderBackgroundColor(OrionInventoryColors::Action::DragMarker)
							.ColorAndOpacity(OrionInventoryColors::Action::DragMarker)
							.BorderImage(FAppStyle::GetBrush("BTEditor.Graph.BTNode.Body"))
							.Visibility(this, &SGraphNode_OrionInventoryNode::GetDragOverMarkerVisibility)
							[
								SNew(SBox)
									.HeightOverride(4.f)
							]
					]

					// Blueprint indicator overlay
					+SOverlay::Slot()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Top)
					[
						SNew(SImage)
							.Image(FAppStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Blueprint")))
							.Visibility(this, &SGraphNode_OrionInventoryNode::GetBlueprintIconVisibility)
					]
						
			]
			
				
		];
		// Create comment bubble
		TSharedPtr<SCommentBubble> CommentBubble;
		const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;

		SAssignNew(CommentBubble, SCommentBubble)
			.GraphNode(GraphNode)
			.Text(this, &SGraphNode::GetNodeComment)
			.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
			.ColorAndOpacity(CommentColor)
			.AllowPinning(true)
			.EnableTitleBarBubble(true)
			.EnableBubbleCtrls(true)
			.GraphLOD(this, &SGraphNode::GetCurrentLOD)
			.IsGraphNodeHovered(this, &SGraphNode::IsHovered);

		GetOrAddSlot(ENodeZone::TopCenter)
			.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
			.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
			.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
			.VAlign(VAlign_Top)
			[
				CommentBubble.ToSharedRef()
			];

		ErrorReporting = ErrorText;
		ErrorReporting->SetError(ErrorMsg);
	
	CreatePinWidgets();
}

void SGraphNode_OrionInventoryNode::OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	// Is someone dragging a node?
	TSharedPtr<FDragNode> DragConnectionOp = DragDropEvent.GetOperationAs<FDragNode>();
	if (DragConnectionOp.IsValid())
	{
		// Inform the Drag and Drop operation that we are hovering over this node.
		TSharedPtr<SGraphNode> SubNode = GetSubNodeUnderCursor(MyGeometry, DragDropEvent);
		DragConnectionOp->SetHoveredNode(SubNode.IsValid() ? SubNode : SharedThis(this));

		UOrionInventoryEdNode* TestNode = Cast<UOrionInventoryEdNode>(GraphNode);
		if (DragConnectionOp->IsValidOperation() && TestNode && TestNode->IsSubNode())
		{
			SetDragMarker(true);
		}
	}

	SGraphNode::OnDragEnter(MyGeometry, DragDropEvent);
}

FReply SGraphNode_OrionInventoryNode::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	// Is someone dragging a node?
	TSharedPtr<FDragNode> DragConnectionOp = DragDropEvent.GetOperationAs<FDragNode>();
	if (DragConnectionOp.IsValid())
	{
		// Inform the Drag and Drop operation that we are hovering over this node.
		TSharedPtr<SGraphNode> SubNode = GetSubNodeUnderCursor(MyGeometry, DragDropEvent);
		DragConnectionOp->SetHoveredNode(SubNode.IsValid() ? SubNode : SharedThis(this));
	}
	return SGraphNode::OnDragOver(MyGeometry, DragDropEvent);
}

void SGraphNode_OrionInventoryNode::OnDragLeave(const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FDragNode> DragConnectionOp = DragDropEvent.GetOperationAs<FDragNode>();
	if (DragConnectionOp.IsValid())
	{
		// Inform the Drag and Drop operation that we are not hovering any pins
		DragConnectionOp->SetHoveredNode(TSharedPtr<SGraphNode>(NULL));
	}

	SetDragMarker(false);
	SGraphNode::OnDragLeave(DragDropEvent);
}

FReply SGraphNode_OrionInventoryNode::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	SetDragMarker(false);

	TSharedPtr<FDragInventoryGraphNode> DragNodeOp = DragDropEvent.GetOperationAs<FDragInventoryGraphNode>();
	if (DragNodeOp.IsValid())
	{
		if (!DragNodeOp->IsValidOperation())
		{
			return FReply::Handled();
		}

		const float DragTime = float(FPlatformTime::Seconds() - DragNodeOp->StartTime);
		if (DragTime < 0.25f)
		{
			return FReply::Handled();
		}

		UOrionInventoryEdNode* MyNode = Cast<UOrionInventoryEdNode>(GraphNode);
		if (MyNode == nullptr || MyNode->IsSubNode())
		{
			return FReply::Unhandled();
		}

		const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_DragDropNode", "Drag&Drop Node"));
		bool bReorderOperation = true;

		const TArray< TSharedRef<SGraphNode> >& DraggedNodes = DragNodeOp->GetNodes();
		for (int32 Idx = 0; Idx < DraggedNodes.Num(); Idx++)
		{
			UOrionInventoryEdNode* DraggedNode = Cast<UOrionInventoryEdNode>(DraggedNodes[Idx]->GetNodeObj());
			if (DraggedNode && DraggedNode->ParentNode)
			{
				if (DraggedNode->ParentNode != GraphNode)
				{
					bReorderOperation = false;
				}

				DraggedNode->ParentNode->RemoveSubNode(DraggedNode);
			}
		}

		UOrionInventoryEdNode* DropTargetNode = DragNodeOp->GetDropTargetNode();
		const int32 InsertIndex = MyNode->FindSubNodeDropIndex(DropTargetNode);

		for (int32 Idx = 0; Idx < DraggedNodes.Num(); Idx++)
		{
			UOrionInventoryEdNode* DraggedTestNode = Cast<UOrionInventoryEdNode>(DraggedNodes[Idx]->GetNodeObj());
			DraggedTestNode->Modify();
			DraggedTestNode->ParentNode = MyNode;

			MyNode->Modify();
			MyNode->InsertSubNodeAt(DraggedTestNode, InsertIndex);
		}

		if (bReorderOperation)
		{
			UpdateGraphNode();
		}
		else
		{
			UOrionInventoryEdGraph* MyGraph = MyNode->GetOrionInventoryEdGraph();
			if (MyGraph)
			{
				MyGraph->OnSubNodeDropped();
			}
		}
	}

	return SGraphNode::OnDrop(MyGeometry, DragDropEvent);
}


FReply SGraphNode_OrionInventoryNode::OnMouseMove(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && !(GEditor->bIsSimulatingInEditor || GEditor->PlayWorld))
	{
		//if we are holding mouse over a subnode
		UOrionInventoryEdNode* TestNode = Cast<UOrionInventoryEdNode>(GraphNode);
		if (TestNode && TestNode->IsSubNode())
		{
			const TSharedRef<SGraphPanel>& Panel = GetOwnerPanel().ToSharedRef();
			const TSharedRef<SGraphNode>& Node = SharedThis(this);
			return FReply::Handled().BeginDragDrop(FDragInventoryGraphNode::New(Panel, Node));
		}
	}

	if (!MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && bDragMarkerVisible)
	{
		SetDragMarker(false);
	}

	return FReply::Unhandled();
}

TSharedRef<SGraphNode> SGraphNode_OrionInventoryNode::GetNodeUnderMouse(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	TSharedPtr<SGraphNode> SubNode = GetSubNodeUnderCursor(MyGeometry, MouseEvent);
	return SubNode.IsValid() ? SubNode.ToSharedRef() : StaticCastSharedRef<SGraphNode>(AsShared());
}

void SGraphNode_OrionInventoryNode::MoveTo(const FVector2D& NewPosition, FNodeSet& NodeFilter, bool bMarkDirty)
{
	SGraphNode::MoveTo(NewPosition, NodeFilter, bMarkDirty);

	// keep node order (defined by linked pins) up to date with actual positions
	// this function will keep spamming on every mouse move update
	UOrionInventoryEdNode* InventoryEdNode = Cast<UOrionInventoryEdNode>(GraphNode);
	if (InventoryEdNode && !InventoryEdNode->IsSubNode())
	{
		UOrionInventoryEdGraph* InventoryEdGraph = InventoryEdNode->GetOrionInventoryEdGraph();
		if (InventoryEdGraph)
		{
			for (int32 Idx = 0; Idx < InventoryEdNode->Pins.Num(); Idx++)
			{
				UEdGraphPin* Pin = InventoryEdNode->Pins[Idx];
				if (Pin && Pin->Direction == EGPD_Input && Pin->LinkedTo.Num() == 1)
				{
					UEdGraphPin* ParentPin = Pin->LinkedTo[0];
					if (ParentPin)
					{
						InventoryEdGraph->RebuildChildOrder(ParentPin->GetOwningNode());
					}
				}
			}
		}
	}
}

TArray<FOverlayWidgetInfo> SGraphNode_OrionInventoryNode::GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const
{
	TArray<FOverlayWidgetInfo> Widgets;

	check(NodeBody.IsValid());
	check(IndexOverlay.IsValid());

	FVector2D Origin(0.0f, 0.0f);
	

	FOverlayWidgetInfo Overlay(IndexOverlay);
	Overlay.OverlayOffset = FVector2D(WidgetSize.X - (IndexOverlay->GetDesiredSize().X * 0.5f), Origin.Y);
	Widgets.Add(Overlay);

	

	return Widgets;
}

void SGraphNode_OrionInventoryNode::CreatePinWidgets()
{
	UOrionInventoryEdNode* StateNode = CastChecked<UOrionInventoryEdNode>(GraphNode);

	for (int32 PinIdx = 0; PinIdx < StateNode->Pins.Num(); PinIdx++)
	{
		UEdGraphPin* MyPin = StateNode->Pins[PinIdx];
		if (!MyPin->bHidden)
		{
			TSharedPtr<SGraphPin> NewPin = SNew(SInventorySystemPin, MyPin)
				.ToolTipText(this, &SGraphNode_OrionInventoryNode::GetPinTooltip, MyPin);

			AddPin(NewPin.ToSharedRef());
		}
	}
}

TSharedPtr<SToolTip> SGraphNode_OrionInventoryNode::GetComplexTooltip()
{
	const UOrionInventoryEdNode* InventoryEdNode = Cast<UOrionInventoryEdNode>(GraphNode);
	const bool bHasErrors = InventoryEdNode && InventoryEdNode->HasErrors();

	if (!bHasErrors)
	{
	}
	return IDocumentation::Get()->CreateToolTip(TAttribute<FText>(this, &SGraphNode::GetNodeTooltip), NULL, GraphNode->GetDocumentationLink(), GraphNode->GetDocumentationExcerptName());

}

void SGraphNode_OrionInventoryNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));

	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();
	const bool bAdvancedParameter = PinObj && PinObj->bAdvancedView;
	if (bAdvancedParameter)
	{
		PinToAdd->SetVisibility(TAttribute<EVisibility>(PinToAdd, &SGraphPin::IsPinVisibleAsAdvanced));
	}

	TSharedPtr<SVerticalBox> PinBox;
	if (PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Input)
	{
		PinBox = LeftNodeBox;
		InputPins.Add(PinToAdd);
	}
	else // Direction == EEdGraphPinDirection::EGPD_Output
	{
		PinBox = RightNodeBox;
		OutputPins.Add(PinToAdd);
	}

	if (PinBox)
	{
		PinBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillHeight(1.0f)
			//.Padding(6.0f, 0.0f)
			[
				PinToAdd
			];
	}
}

void SGraphNode_OrionInventoryNode::SetOwner(const TSharedRef<SGraphPanel>& OwnerPanel)
{
	SGraphNode::SetOwner(OwnerPanel);

	for (auto& ChildWidget : SubNodes)
	{
		if (ChildWidget.IsValid())
		{
			ChildWidget->SetOwner(OwnerPanel);
			OwnerPanel->AttachGraphEvents(ChildWidget);
		}
	}
}

bool SGraphNode_OrionInventoryNode::IsNameReadOnly() const
{
	UOrionInventoryEdNode* InventoryEdNode = Cast<UOrionInventoryEdNode>(GraphNode);
	UDialogBuilderNode* InventoryNode = InventoryEdNode ? Cast<UDialogBuilderNode>(InventoryEdNode->NodeInstance) : nullptr;

	UDialogBuilderGraph* InventoryGraph = InventoryNode ? Cast<UDialogBuilderGraph>(InventoryNode->GetOwningDialogGraph()) : nullptr;

	return (InventoryEdNode && InventoryNode && InventoryGraph) &&
		(!GetDefault<UDialogBuilderSetting>()->bCanRenameNode || !InventoryNode->IsNameEditable()) || SGraphNode::IsNameReadOnly();
}
FReply SGraphNode_OrionInventoryNode::OnMouseDown(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent)
{
	UOrionInventoryEdNode* TestNode = Cast<UOrionInventoryEdNode>(GraphNode);
	if (TestNode && TestNode->IsSubNode())
	{
		GetOwnerPanel()->SelectionManager.ClickedOnNode(GraphNode, MouseEvent);
		return FReply::Handled();
	}

	return FReply::Unhandled();
}
FReply SGraphNode_OrionInventoryNode::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	return SGraphNode::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
}
FText SGraphNode_OrionInventoryNode::GetDescription() const
{
	UOrionInventoryEdNode* InventoryEdNode = CastChecked<UOrionInventoryEdNode>(GraphNode);
	return InventoryEdNode ? InventoryEdNode->GetDescription() : FText::GetEmpty();
}
EVisibility SGraphNode_OrionInventoryNode::GetDescriptionVisibility() const
{
	// LOD this out once things get too small
	TSharedPtr<SGraphPanel> MyOwnerPanel = GetOwnerPanel();
	return (!MyOwnerPanel.IsValid() || MyOwnerPanel->GetCurrentLOD() > EGraphRenderingLOD::LowDetail) ? EVisibility::Visible : EVisibility::Collapsed;

}

EVisibility SGraphNode_OrionInventoryNode::GetEventsVisibility() const
{
	UOrionInventoryEdNode* InventoryEdNode = Cast<UOrionInventoryEdNode>(GraphNode);
	if (InventoryEdNode && InventoryEdNode->Events.Num())
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}
EVisibility SGraphNode_OrionInventoryNode::GetDecoratorVisibility() const
{
	UOrionInventoryEdNode* InventoryEdNode = Cast<UOrionInventoryEdNode>(GraphNode);
	if (InventoryEdNode && InventoryEdNode->Decorators.Num())
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

void SGraphNode_OrionInventoryNode::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SGraphNode::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

void SGraphNode_OrionInventoryNode::AddDecorator(TSharedPtr<SGraphNode> DecoratorWidget)
{
	DecoratorsBox->AddSlot()
		.AutoHeight()
		[
			DecoratorWidget.ToSharedRef()
		];

	DecoratorWidgets.Add(DecoratorWidget);
	AddSubNode(DecoratorWidget);
}

void SGraphNode_OrionInventoryNode::AddEvent(TSharedPtr<SGraphNode> EventWidget)
{
	EventsBox->AddSlot().AutoHeight()
		[
			EventWidget.ToSharedRef()
		];
	EventsWidgets.Add(EventWidget);
	AddSubNode(EventWidget);
}

void SGraphNode_OrionInventoryNode::AddSubNode(TSharedPtr<SGraphNode> SubNodeWidget)
{
	SubNodes.Add(SubNodeWidget);
}

TSharedPtr<SGraphNode> SGraphNode_OrionInventoryNode::GetSubNodeUnderCursor(const FGeometry& WidgetGeometry, const FPointerEvent& MouseEvent)
{
	TSharedPtr<SGraphNode> ResultNode;

	// We just need to find the one WidgetToFind among our descendants.
	TSet< TSharedRef<SWidget> > SubWidgetsSet;
	for (int32 i = 0; i < SubNodes.Num(); i++)
	{
		SubWidgetsSet.Add(SubNodes[i].ToSharedRef());
	}

	TMap<TSharedRef<SWidget>, FArrangedWidget> Result;
	FindChildGeometries(WidgetGeometry, SubWidgetsSet, Result);

	if (Result.Num() > 0)
	{
		FArrangedChildren ArrangedChildren(EVisibility::Visible);
		Result.GenerateValueArray(ArrangedChildren.GetInternalArray());

		const int32 HoveredIndex = SWidget::FindChildUnderMouse(ArrangedChildren, MouseEvent);
		if (HoveredIndex != INDEX_NONE)
		{
			ResultNode = StaticCastSharedRef<SGraphNode>(ArrangedChildren[HoveredIndex].Widget);
		}
	}

	return ResultNode;
}

bool SGraphNode_OrionInventoryNode::OnVerifyNameTextChanged(const FText& InText, FText& OutErrorMessage)
{
	FString FinalString = InText.ToString().TrimStartAndEnd();

	FName OriginalName;

	bool bValid(true);

	if ((GetEditableNodeTitle() != FinalString) && OnVerifyTextCommit.IsBound())
	{
		OutErrorMessage = LOCTEXT("RenameFailed_NotValid", "Name Not Valid.");
        bValid = OnVerifyTextCommit.Execute(FText::FromString(FinalString), GraphNode, OutErrorMessage);
	}

	UOrionInventoryEdNode* InventoryEdNode = Cast<UOrionInventoryEdNode>(GraphNode);
	UDialogBuilderNode* InventoryNode = InventoryEdNode ? Cast<UDialogBuilderNode>(InventoryEdNode->NodeInstance) : nullptr;
	UOrionInventoryEdGraph* InventoryEdGraph = InventoryEdNode ? InventoryEdNode->GetOrionInventoryEdGraph() : nullptr;

	if (InventoryNode && InventoryEdGraph)
	{
		OriginalName = InventoryNode->ID;
		for (auto& Node : InventoryEdGraph->Nodes)
		{
			UOrionInventoryEdNode* NodeEdNode = Cast<UOrionInventoryEdNode>(Node);
			if (NodeEdNode && NodeEdNode->NodeInstance)
			{
				UDialogBuilderNode* NodeInstance = Cast<UDialogBuilderNode>(NodeEdNode->NodeInstance);
				if (NodeInstance && NodeInstance != InventoryNode)
				{
					if (FName(FinalString) == OriginalName)
					{
						return true;
					}
					if (FName(FinalString) == NodeInstance->ID)
					{
						OutErrorMessage = LOCTEXT("RenameFailed_NotValid", "This ID is already used in another Node.");
						return false;
					}
				}
			}
		}
	}


	return bValid;
}
bool SGraphNode_OrionInventoryNode::UseLowDetailNodeTitles() const
{
	if (InlineEditableText.IsValid())
	{
		if (const SGraphPanel* MyOwnerPanel = GetOwnerPanel().Get())
		{
			return (MyOwnerPanel->GetCurrentLOD() <= EGraphRenderingLOD::LowestDetail) && !InlineEditableText->IsInEditMode();
		}
	}

	return false;
}
FText SGraphNode_OrionInventoryNode::GetPinTooltip(UEdGraphPin* GraphPinObj) const
{
	FText HoverText = FText::GetEmpty();

	check(GraphPinObj != nullptr);
	UEdGraphNode* OwningGraphNode = GraphPinObj->GetOwningNode();
	if (OwningGraphNode != nullptr)
	{
		FString HoverStr;
		OwningGraphNode->GetPinHoverText(*GraphPinObj, /*out*/HoverStr);
		if (!HoverStr.IsEmpty())
		{
			HoverText = FText::FromString(HoverStr);
		}
	}

	return HoverText;

}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SGraphNode_OrionInventoryNode::OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo)
{
	SGraphNode::OnNameTextCommited(InText, CommitInfo);

	const FString NewNameString = InText.ToString().TrimStartAndEnd();
	const FName NewName = *NewNameString;
	FName OriginalName;

	UOrionInventoryEdNode* InventoryEdNode = Cast<UOrionInventoryEdNode>(GraphNode);
	UDialogBuilderNode* InventoryNode = InventoryEdNode ? Cast<UDialogBuilderNode>(InventoryEdNode->NodeInstance) : nullptr;

	if (InventoryEdNode && InventoryNode)
	{
		OriginalName = InventoryNode->ID;

		//Finalize Node and Update GraphNode
		const FScopedTransaction Transaction(LOCTEXT("InventorySystemEditorRenameNode", "Inventory System Editor: Rename Node"));
		InventoryEdNode->Modify();
		InventoryNode->Modify();
		InventoryNode->SetNodeTitle(FText::FromString(NewNameString));
		UpdateGraphNode();

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

		TArray<FAssetData> InventoryGraphDataArray;
		AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UDialogBuilderGraph::StaticClass()), InventoryGraphDataArray);

		for (const FAssetData& AssetData : InventoryGraphDataArray)
		{
			UDialogBuilderGraph* InventoryGraph = Cast<UDialogBuilderGraph>(AssetData.GetAsset());
			if (InventoryGraph)
			{
				
			}
		}
	}
}

FSlateColor SGraphNode_OrionInventoryNode::GetBackgroundColor() const
{
	UOrionInventoryEdNode* InventoryEdNode = CastChecked<UOrionInventoryEdNode>(GraphNode);

	FLinearColor NodeColor = OrionInventoryColors::NodeBody::Default;
	if (InventoryEdNode && InventoryEdNode->HasErrors())
	{
		NodeColor = OrionInventoryColors::NodeBody::Error;
	}
	else if (!InventoryEdNode->IsSubNode())
	{
		NodeColor = OrionInventoryColors::NodeBorder::NoHighlight;
	}
	else if (InventoryEdNode)
	{
		NodeColor = InventoryEdNode->GetBackgroundColor();
	}

	return NodeColor;
}

FSlateColor SGraphNode_OrionInventoryNode::GetBorderBackgroundColor() const
{
	UOrionInventoryEdNode* InventoryEdNode = Cast<UOrionInventoryEdNode>(GraphNode);
	UOrionInventoryEdNode* InventoryParentNode = InventoryEdNode ? Cast<UOrionInventoryEdNode>(InventoryEdNode->ParentNode) : nullptr;
	const bool bSelectedSubNode = InventoryParentNode && GetOwnerPanel()->SelectionManager.SelectedNodes.Contains(GraphNode);


	UDialogBuilderNode* InventoryNode = InventoryEdNode ? Cast<UDialogBuilderNode>(InventoryEdNode->NodeInstance) : nullptr;

	return bSelectedSubNode ? OrionInventoryColors::NodeBorder::Selected :
		InventoryEdNode && !InventoryEdNode->IsSubNode() ? InventoryEdNode->GetBackgroundColor() :
		OrionInventoryColors::NodeBorder::Inactive;

}

FText SGraphNode_OrionInventoryNode::GetNodeIDText() const
{
	UOrionInventoryEdNode* InventoryEdNode = Cast<UOrionInventoryEdNode>(GraphNode);
	UDialogBuilderNode* InventoryNode = InventoryEdNode ? Cast<UDialogBuilderNode>(InventoryEdNode->NodeInstance) : nullptr;
	if (InventoryNode)
	{
		return FText::FromString("ID: " + InventoryNode->ID.ToString());
	}
	return FText::FromString("ID: None");
}

EVisibility SGraphNode_OrionInventoryNode::GetNodeIDVisibility() const
{
	UOrionInventoryEdNode_Root* RootInventoryEdNode = Cast<UOrionInventoryEdNode_Root>(GraphNode);
	if (RootInventoryEdNode)
	{
		return EVisibility::Collapsed;
	}
	return EVisibility::Visible;
}

EVisibility SGraphNode_OrionInventoryNode::GetTitleVisibility() const
{
	return EVisibility::Visible;
}

EVisibility SGraphNode_OrionInventoryNode::GetParentNodeVisibility() const
{
	UOrionInventoryEdNode* TestNode = Cast<UOrionInventoryEdNode>(GraphNode);
	if (TestNode && TestNode->IsSubNode())
	{
		return EVisibility::Collapsed;
	}
	return EVisibility::Visible;
}

EVisibility SGraphNode_OrionInventoryNode::GetSubNodeVisibility() const
{
	UOrionInventoryEdNode* TestNode = Cast<UOrionInventoryEdNode>(GraphNode);
	if (TestNode && TestNode->IsSubNode())
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility SGraphNode_OrionInventoryNode::GetDragOverMarkerVisibility() const
{
	return bDragMarkerVisible ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SGraphNode_OrionInventoryNode::GetDebuggerSearchFailedMarkerVisibility() const
{
	return EVisibility();
}

const FSlateBrush* SGraphNode_OrionInventoryNode::GetNameIcon() const
{
	UOrionInventoryEdNode* InventoryEdNode = Cast<UOrionInventoryEdNode>(GraphNode);
	if (InventoryEdNode != nullptr)
	{
		return FAppStyle::GetBrush(InventoryEdNode->GetNameIcon());
	}
	return FAppStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Icon"));
}

void SGraphNode_OrionInventoryNode::SetDragMarker(bool bEnabled)
{
	bDragMarkerVisible = bEnabled;
}

EVisibility SGraphNode_OrionInventoryNode::GetBlueprintIconVisibility() const
{
	UOrionInventoryEdNode* InventoryEdNode = CastChecked<UOrionInventoryEdNode>(GraphNode);
	const bool bCanShowIcon = (InventoryEdNode != nullptr && InventoryEdNode->UsesBlueprint());

	// LOD this out once things get too small
	TSharedPtr<SGraphPanel> MyOwnerPanel = GetOwnerPanel();
	return (bCanShowIcon && (!MyOwnerPanel.IsValid() || MyOwnerPanel->GetCurrentLOD() > EGraphRenderingLOD::LowDetail)) ? EVisibility::Visible : EVisibility::Collapsed;

}

EVisibility SGraphNode_OrionInventoryNode::GetIndexVisibility() const
{
	// always hide the index on the root node
	if (GraphNode->IsA(UOrionInventoryEdNode_Root::StaticClass()))
	{
		return EVisibility::Collapsed;
	}

	UOrionInventoryEdNode* InventoryEdNode = CastChecked<UOrionInventoryEdNode>(GraphNode);
	UEdGraphPin* MyInputPin = InventoryEdNode->GetInputPin();
	UEdGraphPin* MyParentOutputPin = NULL;
	if (MyInputPin != NULL && MyInputPin->LinkedTo.Num() > 0)
	{
		MyParentOutputPin = MyInputPin->LinkedTo[0];
	}

	
	// LOD this out once things get too small
	TSharedPtr<SGraphPanel> MyOwnerPanel = GetOwnerPanel();

	UOrionInventoryEdGraph* InventoryEdGraph = Cast<UOrionInventoryEdGraph>(GraphNode->GetGraph());

	FGraphPanelSelectionSet SelectedNodes;
	if (InventoryEdGraph && InventoryEdGraph->SEditorGraph)
	{
		SelectedNodes = InventoryEdGraph->SEditorGraph->GetSelectedNodes();
	}

	UDialogBuilderNode* FirstSelectedNode = nullptr;
	if (SelectedNodes.Num() > 0)
	{
		// Get the first element of the SelectedNodes set
		UOrionInventoryEdNode* SelectedInventoryEdNode = Cast<UOrionInventoryEdNode>(*SelectedNodes.CreateConstIterator());
		FirstSelectedNode = SelectedInventoryEdNode ? Cast<UDialogBuilderNode>(SelectedInventoryEdNode->NodeInstance) : nullptr;
	}

	UDialogBuilderNode* InventoryNode = Cast<UDialogBuilderNode>(InventoryEdNode->NodeInstance);

	// Visible if ((we are in PIE || if we have siblings) && (SelectedNodes only one && Matching parent with selected nodes))
	const bool bCanShowIndex = (ShouldShowExecutionIndex() || (MyParentOutputPin && MyParentOutputPin->LinkedTo.Num() > 1)) && (SelectedNodes.Num() == 1 && (FirstSelectedNode && InventoryNode) && (InventoryNode->ParentNodes.Contains(FirstSelectedNode)));

	return (bCanShowIndex && (!MyOwnerPanel.IsValid() || MyOwnerPanel->GetCurrentLOD() > EGraphRenderingLOD::LowDetail)) ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SGraphNode_OrionInventoryNode::GetIndexText() const
{
	UOrionInventoryEdNode* InventoryEdNode = Cast<UOrionInventoryEdNode>(GraphNode);
	UEdGraphPin* MyInputPin = InventoryEdNode->GetInputPin();
	UEdGraphPin* MyParentOutputPin = NULL;
	if (MyInputPin != NULL && MyInputPin->LinkedTo.Num() > 0)
	{
		MyParentOutputPin = MyInputPin->LinkedTo[0];
	}

	int32 Index = 0;

	UOrionInventoryEdGraph* InventoryEdGraph = Cast<UOrionInventoryEdGraph>(GraphNode->GetGraph());

	FGraphPanelSelectionSet SelectedNodes;
	if (InventoryEdGraph && InventoryEdGraph->SEditorGraph)
	{
		SelectedNodes = InventoryEdGraph->SEditorGraph->GetSelectedNodes();
	}

	UDialogBuilderNode* FirstSelectedNode = nullptr;
	if (SelectedNodes.Num() > 0)
	{
		// Get the first element of the SelectedNodes set
		UOrionInventoryEdNode* SelectedInventoryEdNode = Cast<UOrionInventoryEdNode>(*SelectedNodes.CreateConstIterator());
		FirstSelectedNode = SelectedInventoryEdNode ? Cast<UDialogBuilderNode>(SelectedInventoryEdNode->NodeInstance) : nullptr;
	}

	UDialogBuilderNode* InventoryNode = Cast<UDialogBuilderNode>(InventoryEdNode->NodeInstance);

	if (InventoryNode->ParentNodes.Contains(FirstSelectedNode))
	{
		for (int i = 0; i < FirstSelectedNode->ChildrenNodes.Num(); i++)
		{
			//Select this index if node matches with the selected node childrens
			if (FirstSelectedNode->ChildrenNodes[i] == InventoryNode)
			{
				Index = i + 1;//Index start from 1
				break;
			}
		}
	}
	
	
	return FText::AsNumber(Index);
}

FText SGraphNode_OrionInventoryNode::GetIndexTooltipText() const
{
	if (ShouldShowExecutionIndex())
	{
		return LOCTEXT("ExecutionIndexTooltip", "Execution index: this shows the order in which nodes are executed.");
	}
	else
	{
		return LOCTEXT("ChildIndexTooltip", "Child index: this shows the order in which child nodes are executed.");
	}
}

FSlateColor SGraphNode_OrionInventoryNode::GetIndexColor(bool bHovered) const
{
	const bool bHighlightHover = bHovered;

	static const FName HoveredColor("BTEditor.Graph.BTNode.Index.HoveredColor");
	static const FName DefaultColor("BTEditor.Graph.BTNode.Index.Color");

	return bHighlightHover ? FAppStyle::Get().GetSlateColor(HoveredColor) : FAppStyle::Get().GetSlateColor(DefaultColor);
}

void SGraphNode_OrionInventoryNode::OnIndexHoverStateChanged(bool bHovered)
{
}

TSharedRef<FDragInventoryGraphNode> FDragInventoryGraphNode::New(const TSharedRef<SGraphPanel>& InGraphPanel, const TSharedRef<SGraphNode>& InDraggedNode)
{
	TSharedRef<FDragInventoryGraphNode> Operation = MakeShareable(new FDragInventoryGraphNode);
	Operation->StartTime = FPlatformTime::Seconds();
	Operation->GraphPanel = InGraphPanel;
	Operation->DraggedNodes.Add(InDraggedNode);
	// adjust the decorator away from the current mouse location a small amount based on cursor size
	Operation->DecoratorAdjust = FSlateApplication::Get().GetCursorSize();
	Operation->Construct();
	return Operation;
}

TSharedRef<FDragInventoryGraphNode> FDragInventoryGraphNode::New(const TSharedRef<SGraphPanel>& InGraphPanel, const TArray<TSharedRef<SGraphNode>>& InDraggedNodes)
{
	TSharedRef<FDragInventoryGraphNode> Operation = MakeShareable(new FDragInventoryGraphNode);
	Operation->StartTime = FPlatformTime::Seconds();
	Operation->GraphPanel = InGraphPanel;
	Operation->DraggedNodes = InDraggedNodes;
	Operation->DecoratorAdjust = FSlateApplication::Get().GetCursorSize();
	Operation->Construct();
	return Operation;
}

UOrionInventoryEdNode* FDragInventoryGraphNode::GetDropTargetNode() const
{
	return Cast<UOrionInventoryEdNode>(GetHoveredNode());
}


#undef LOCTEXT_NAMESPACE

