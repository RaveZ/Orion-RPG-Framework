// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "SGraphNode_DialogBuilderNode.h"
#include "SlateOptMacros.h"
#include "Types/SlateStructs.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SToolTip.h"
#include "SGraphPanel.h"
#include "Components/VerticalBox.h"
#include "SGraphPreviewer.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "SGraphPin.h"
#include "Colors_DialogBuilder.h"
#include "GraphEditorSettings.h"
#include "SCommentBubble.h"
#include "SLevelOfDetailBranchNode.h"
#include "QuestBuilderNode.h"
#include "DialogBuilderEdNode_Root.h"
#include "QuestBuilderNode_Objective.h"
#include "DialogBuilderSetting.h"	
#include "Fonts/SlateFontInfo.h"
#include "Decorator/OrionDecorator.h"
#include "Event/OrionEvent.h"
#include "AssetRegistry/AssetRegistryModule.h"

#define LOCTEXT_NAMESPACE "EdNode_QuestBuilder"

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
class SQuestSystemPin : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SQuestSystemPin) {}
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
			.BorderImage(this, &SQuestSystemPin::GetPinBorder)
			.BorderBackgroundColor(this, &SQuestSystemPin::GetPinColor)
			.OnMouseButtonDown(this, &SQuestSystemPin::OnPinMouseDown)
			.Cursor(this, &SQuestSystemPin::GetPinCursor)
			.Padding(FMargin(5.0f))
		);
	}

protected:
	virtual FSlateColor GetPinColor() const override
	{
		return bIsDiffHighlighted ? DialogBuilderColors::Pin::Diff :
			IsHovered() ? DialogBuilderColors::Pin::Hover : DialogBuilderColors::Pin::Default;
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
class SQuestNodeIndex : public SCompoundWidget
{
public:
	/** Delegate event fired when the hover state of this widget changes */
	DECLARE_DELEGATE_OneParam(FOnHoverStateChanged, bool /* bHovered */);

	/** Delegate used to receive the color of the node, depending on hover state and state of other siblings */
	DECLARE_DELEGATE_RetVal_OneParam(FSlateColor, FOnGetIndexColor, bool /* bHovered */);

	SLATE_BEGIN_ARGS(SQuestNodeIndex) {}
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
							.BorderBackgroundColor(this, &SQuestNodeIndex::GetColor)
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


void SGraphNode_DialogBuilderNode::Construct(const FArguments& InArgs, UDialogBuilderEdNode* InNode)
{
	GraphNode = InNode;
	UpdateGraphNode();
	InNode->SGraphNode = this;
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SGraphNode_DialogBuilderNode::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	const FMargin NodePadding = FMargin(4.f);
	const FMargin NamePadding = FMargin(2);

	TSharedPtr<SErrorText> ErrorText;
	TSharedPtr<SVerticalBox> NodeBody;
	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);
	TWeakPtr<SNodeTitle> WeakNodeTitle = NodeTitle;

	TSharedPtr<STextBlock> DescriptionText;

	UDialogBuilderEdNode* QuestEdNode = Cast<UDialogBuilderEdNode>(GraphNode);
	UQuestBuilderNode* QuestNode = QuestEdNode ? Cast<UQuestBuilderNode>(QuestEdNode->NodeInstance) : nullptr;

	UWorld* World = GEditor->GetEditorWorldContext().World();

	IndexOverlay = SNew(SQuestNodeIndex)
		.ToolTipText(this, &SGraphNode_DialogBuilderNode::GetIndexTooltipText)
		.Visibility(this, &SGraphNode_DialogBuilderNode::GetIndexVisibility)
		.Text(this, &SGraphNode_DialogBuilderNode::GetIndexText)
		.OnHoverStateChanged(this, &SGraphNode_DialogBuilderNode::OnIndexHoverStateChanged)
		.OnGetIndexColor(this, &SGraphNode_DialogBuilderNode::GetIndexColor);
	
	auto SizeBox = SNew(SBox);
	SizeBox->SetMinDesiredWidth(250.f);
	SizeBox->SetMinDesiredHeight(60.f);

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
				.Padding(3.0f)
				.BorderBackgroundColor(this, &SGraphNode_DialogBuilderNode::GetNodeHighlightColor)
				[
					SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
						.Padding(0.0f)
						.BorderBackgroundColor(this, &SGraphNode_DialogBuilderNode::GetBorderBackgroundColor)
						//.OnMouseButtonDown(this, &SGraphNode_DialogBuilderNode::OnMouseDown)
						[
							SNew(SOverlay)
								+ SOverlay::Slot()
								[
									SizeBox
								]
							// Pins and node details
							+ SOverlay::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								[
									SNew(SHorizontalBox)

										// INPUT PIN AREA
										+ SHorizontalBox::Slot()
										.AutoWidth()
										[
											SNew(SBox)
												.MinDesiredHeight(NodePadding.Top)
												[
													SAssignNew(LeftNodeBox, SVerticalBox)
												]
										]

										// STATE NAME AREA
										+ SHorizontalBox::Slot()
										.Padding(FMargin(NodePadding.Left, 0.0f, NodePadding.Right, 0.0f))
										[
											SNew(SVerticalBox)
												+ SVerticalBox::Slot()
												.AutoHeight()
												[
													SNew(SOverlay)
														+ SOverlay::Slot()
														.HAlign(HAlign_Center)
														.VAlign(VAlign_Center)
														.Padding(7.0f)
														[
															SAssignNew(NodeBody, SVerticalBox)

																// Title
																+ SVerticalBox::Slot()
																.AutoHeight()
																[
																	SNew(SVerticalBox)
																		+ SVerticalBox::Slot()
																		.AutoHeight()
																		[
																			SAssignNew(InlineEditableText, SInlineEditableTextBlock)
																				.Style(FAppStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText")
																				.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
																				.OnVerifyTextChanged(this, &SGraphNode_DialogBuilderNode::OnVerifyNameTextChanged)
																				.OnTextCommitted(this, &SGraphNode_DialogBuilderNode::OnNameTextCommited)
																				.IsReadOnly(this, &SGraphNode_DialogBuilderNode::IsNameReadOnly)
																				.IsSelected(this, &SGraphNode_DialogBuilderNode::IsSelectedExclusively)
																				.Clipping(EWidgetClipping::Inherit)
																		]
																		+ SVerticalBox::Slot()
																		.AutoHeight()
																		[
																			NodeTitle.ToSharedRef()
																		]

																]
																// Error Msg
																+ SVerticalBox::Slot()
																.AutoHeight()
																[
																	SAssignNew(ErrorText, SErrorText)
																		.BackgroundColor(this, &SGraphNode_DialogBuilderNode::GetErrorColor)
																		.ToolTipText(this, &SGraphNode_DialogBuilderNode::GetErrorMsgToolTip)
																]
																	
														]
												]
												// Description
												+ SVerticalBox::Slot()
												.AutoHeight()
												.HAlign(HAlign_Fill)
												.VAlign(VAlign_Center)
												[
													SNew(SVerticalBox)
														+ SVerticalBox::Slot()
														.Padding(FMargin(5.0f, 0.0f, 5.0f, 3.0f))
														.AutoHeight()
														[
															// TASK TITLE
															SNew(STextBlock)
																.Visibility(this, &SGraphNode_DialogBuilderNode::GetTaskVisibility)
                                                                .Text(LOCTEXT("TaskLabel", "TASK"))
																.TextStyle(FAppStyle::Get(), TEXT("PhysicsAssetEditor.Tools.Font"))
																.Justification(ETextJustify::Center)
																.Clipping(EWidgetClipping::Inherit)
														]
														+ SVerticalBox::Slot()
														.Padding(FMargin(5.0f, 0.0f, 5.0f, 8.0f))
														.AutoHeight()
														[
															// DESCRIPTION MESSAGE | Including Task Datas
															SNew(STextBlock)
																.Visibility(this, &SGraphNode_DialogBuilderNode::GetDescriptionVisibility)
																.Text(this, &SGraphNode_DialogBuilderNode::GetDescription)
																.Justification(ETextJustify::Center)
																.Clipping(EWidgetClipping::Inherit)
														]
															
												]

												// Events
												+ SVerticalBox::Slot()
												.AutoHeight()
												.HAlign(HAlign_Fill)
												.VAlign(VAlign_Center)
												[
													SNew(SVerticalBox)
														+ SVerticalBox::Slot()
														.Padding(FMargin(5.0f, 0.0f, 5.0f, 3.0f))
														.AutoHeight()
														[
															// EVENT TITLE
															SNew(STextBlock)
																.Visibility(this, &SGraphNode_DialogBuilderNode::GetEventsVisibility)
																.Text(LOCTEXT("TaskLabel", "EVENT"))
																.TextStyle(FAppStyle::Get(), TEXT("PhysicsAssetEditor.Tools.Font"))
																.Justification(ETextJustify::Center)
																.Clipping(EWidgetClipping::Inherit)
														]
														+ SVerticalBox::Slot()
														.Padding(FMargin(5.0f, 0.0f, 5.0f, 8.0f))
														.AutoHeight()
														[
															//Events Text
															SNew(STextBlock)
																.Visibility(this, &SGraphNode_DialogBuilderNode::GetEventsVisibility)
																.Text(this, &SGraphNode_DialogBuilderNode::GetEventsText)
																.Justification(ETextJustify::Center)
																.Clipping(EWidgetClipping::Inherit)
														]

												]

												// Conditions
												+ SVerticalBox::Slot()
												.AutoHeight()
												.HAlign(HAlign_Fill)
												.VAlign(VAlign_Center)
												[
													SNew(SVerticalBox)
														+ SVerticalBox::Slot()
														.Padding(FMargin(5.0f, 0.0f, 5.0f, 3.0f))
														.AutoHeight()
														[
															// BRANCHING CONDITION TITLE
															SNew(STextBlock)
																.Visibility(this, &SGraphNode_DialogBuilderNode::GetConditionsVisibility)
																.Text(LOCTEXT("TaskLabel", "BRANCHING CONDITION"))
																.TextStyle(FAppStyle::Get(), TEXT("PhysicsAssetEditor.Tools.Font"))
																.Justification(ETextJustify::Center)
																.Clipping(EWidgetClipping::Inherit)
														]
														+ SVerticalBox::Slot()
														.Padding(FMargin(5.0f, 0.0f, 5.0f, 8.0f))
														.AutoHeight()
														[
															//Events Text
															SNew(STextBlock)
																.Visibility(this, &SGraphNode_DialogBuilderNode::GetConditionsVisibility)
																.Text(this, &SGraphNode_DialogBuilderNode::GetConditionsText)
																.Justification(ETextJustify::Center)
																.Clipping(EWidgetClipping::Inherit)
														]

												]

										]

										// OUTPUT PIN AREA
										+ SHorizontalBox::Slot()
										.AutoWidth()
										[
											SNew(SBox)
												.MinDesiredHeight(NodePadding.Bottom)
												[
													SAssignNew(RightNodeBox, SVerticalBox)

												]
										]
								]


								// Blueprint indicator overlay
								+SOverlay::Slot()
								.HAlign(HAlign_Left)
								.VAlign(VAlign_Top)
								[
									SNew(SImage)
										.Image(FAppStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Blueprint")))
										.Visibility(this, &SGraphNode_DialogBuilderNode::GetBlueprintIconVisibility)
								]
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

TArray<FOverlayWidgetInfo> SGraphNode_DialogBuilderNode::GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const
{
	TArray<FOverlayWidgetInfo> Widgets;

	check(IndexOverlay.IsValid());

	FVector2D Origin(0.0f, 0.0f);
	FOverlayWidgetInfo Overlay(IndexOverlay);
	Overlay.OverlayOffset = FVector2D(WidgetSize.X - (IndexOverlay->GetDesiredSize().X * 0.5f), Origin.Y);
	Widgets.Add(Overlay);

	return Widgets;
}

void SGraphNode_DialogBuilderNode::CreatePinWidgets()
{
	UDialogBuilderEdNode* StateNode = CastChecked<UDialogBuilderEdNode>(GraphNode);

	for (int32 PinIdx = 0; PinIdx < StateNode->Pins.Num(); PinIdx++)
	{
		UEdGraphPin* MyPin = StateNode->Pins[PinIdx];
		if (!MyPin->bHidden)
		{
			TSharedPtr<SGraphPin> NewPin = SNew(SQuestSystemPin, MyPin);

			AddPin(NewPin.ToSharedRef());
		}
	}
}

void SGraphNode_DialogBuilderNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
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

bool SGraphNode_DialogBuilderNode::IsNameReadOnly() const
{
	UDialogBuilderEdNode* QuestEdNode = Cast<UDialogBuilderEdNode>(GraphNode);
	UQuestBuilderNode* QuestNode = QuestEdNode ? Cast<UQuestBuilderNode>(QuestEdNode->NodeInstance) : nullptr;

	UQuestBuilderGraph* QuestGraph = QuestNode ? Cast<UQuestBuilderGraph>(QuestNode->GetOwningQuestGraph()) : nullptr;

	return (QuestEdNode && QuestNode && QuestGraph) &&
		(!GetDefault<UDialogBuilderSetting>()->bCanRenameNode || !QuestNode->IsNameEditable()) || SGraphNode::IsNameReadOnly();
}
FText SGraphNode_DialogBuilderNode::GetDescription() const
{
	UDialogBuilderEdNode* QuestEdNode = CastChecked<UDialogBuilderEdNode>(GraphNode);
	return QuestEdNode ? QuestEdNode->GetDescription() : FText::GetEmpty();
}
EVisibility SGraphNode_DialogBuilderNode::GetDescriptionVisibility() const
{
	// LOD this out once things get too small
	TSharedPtr<SGraphPanel> MyOwnerPanel = GetOwnerPanel();
	return (!MyOwnerPanel.IsValid() || MyOwnerPanel->GetCurrentLOD() > EGraphRenderingLOD::LowDetail) ? EVisibility::Visible : EVisibility::Collapsed;

}
EVisibility SGraphNode_DialogBuilderNode::GetTaskVisibility() const
{
	UDialogBuilderEdNode* QuestEdNode = Cast<UDialogBuilderEdNode>(GraphNode);
	UQuestBuilderNode* QuestNode = QuestEdNode ? Cast<UQuestBuilderNode>(QuestEdNode->NodeInstance) : nullptr;

	if (UQuestBuilderNode_Objective* QuestSystemNodeObjective = Cast<UQuestBuilderNode_Objective>(QuestNode))
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}
FText SGraphNode_DialogBuilderNode::GetEventsText() const
{
	UDialogBuilderEdNode* QuestEdNode = CastChecked<UDialogBuilderEdNode>(GraphNode);
	return QuestEdNode ? QuestEdNode->GetEventsText() : FText::GetEmpty();
}
EVisibility SGraphNode_DialogBuilderNode::GetEventsVisibility() const
{
	UDialogBuilderEdNode* QuestEdNode = Cast<UDialogBuilderEdNode>(GraphNode);
	UQuestBuilderNode* QuestNode = QuestEdNode ? Cast<UQuestBuilderNode>(QuestEdNode->NodeInstance) : nullptr;
	if (QuestNode && !QuestNode->Events.IsEmpty())
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}
FText SGraphNode_DialogBuilderNode::GetConditionsText() const
{
	UDialogBuilderEdNode* QuestEdNode = CastChecked<UDialogBuilderEdNode>(GraphNode);
	return QuestEdNode ? QuestEdNode->GetConditionsText() : FText::GetEmpty();
}
EVisibility SGraphNode_DialogBuilderNode::GetConditionsVisibility() const
{
	UDialogBuilderEdNode* QuestEdNode = Cast<UDialogBuilderEdNode>(GraphNode);
	UQuestBuilderNode* QuestNode = QuestEdNode ? Cast<UQuestBuilderNode>(QuestEdNode->NodeInstance) : nullptr;
	if (QuestNode && !QuestNode->Conditions.IsEmpty())
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

bool SGraphNode_DialogBuilderNode::OnVerifyNameTextChanged(const FText& InText, FText& OutErrorMessage)
{
	FString FinalString = InText.ToString().TrimStartAndEnd();

	FName OriginalName;

	bool bValid(true);

	if ((GetEditableNodeTitle() != FinalString) && OnVerifyTextCommit.IsBound())
	{
		OutErrorMessage = LOCTEXT("RenameFailed_NotValid", "Name Not Valid.");
        bValid = OnVerifyTextCommit.Execute(FText::FromString(FinalString), GraphNode, OutErrorMessage);
	}

	UDialogBuilderEdNode* QuestEdNode = Cast<UDialogBuilderEdNode>(GraphNode);
	UQuestBuilderNode* QuestNode = QuestEdNode ? Cast<UQuestBuilderNode>(QuestEdNode->NodeInstance) : nullptr;

	if (QuestNode)
	{
		OriginalName = QuestNode->ID;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> QuestGraphDataArray;
	AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UQuestBuilderGraph::StaticClass()), QuestGraphDataArray);

	for (const FAssetData& AssetData : QuestGraphDataArray)
	{
		UQuestBuilderGraph* QuestGraph = Cast<UQuestBuilderGraph>(AssetData.GetAsset());
		if (QuestGraph)
		{
			if (FName(FinalString) == OriginalName)
			{
				return true;
			}
			UQuestBuilderNode* FoundedDuplicateNode = QuestGraph->NodeIDMap.FindRef(FName(FinalString));
			if(FoundedDuplicateNode)
			{
				OutErrorMessage = LOCTEXT("RenameFailed_NotValid", "This ID is already used in another Node.");
				return false;
			}
		}
	}


	return bValid;
}
FText SGraphNode_DialogBuilderNode::GetPinTooltip(UEdGraphPin* GraphPinObj) const
{
	return FText();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SGraphNode_DialogBuilderNode::OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo)
{
	SGraphNode::OnNameTextCommited(InText, CommitInfo);

	const FString NewNameString = InText.ToString().TrimStartAndEnd();
	const FName NewName = *NewNameString;
	FName OriginalName;

	UDialogBuilderEdNode* QuestEdNode = Cast<UDialogBuilderEdNode>(GraphNode);
	UQuestBuilderNode* QuestNode = QuestEdNode ? Cast<UQuestBuilderNode>(QuestEdNode->NodeInstance) : nullptr;

	if (QuestEdNode && QuestNode)
	{
		UDialogBuilderEdGraph* QuestEdGraph = QuestEdNode->GetQuestBuilderEdGraph();
		OriginalName = QuestNode->ID;

		//Finalize Node and Update GraphNode
		const FScopedTransaction Transaction(LOCTEXT("QuestSystemEditorRenameNode", "Dialog System Editor: Rename Node"));
		QuestEdNode->Modify();
		QuestNode->Modify();
		QuestNode->SetNodeTitle(FText::FromString(NewNameString));
		UpdateGraphNode();

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

		TArray<FAssetData> QuestGraphDataArray;
		AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UQuestBuilderGraph::StaticClass()), QuestGraphDataArray);

		for (const FAssetData& AssetData : QuestGraphDataArray)
		{
			UQuestBuilderGraph* QuestGraph = Cast<UQuestBuilderGraph>(AssetData.GetAsset());
			if (QuestGraph)
			{
				for (auto& Dialog : QuestGraph->QuestList)
				{
					for (auto& QuestCondition : Dialog->Conditions)
					{
						if (QuestCondition && QuestCondition->NodeID == OriginalName)
						{
							QuestCondition->NodeID = NewName;
							QuestCondition->Modify();
						}
					}
					for (auto& Node : Dialog->AllNodes)
					{
						for (auto& NodeCondition : Node->Conditions)
						{
							if (NodeCondition && NodeCondition->NodeID == OriginalName)
							{
								NodeCondition->NodeID = NewName;
								NodeCondition->Modify();
							}
						}
						for (auto& NodeEvent : Node->Events)
						{
							if (NodeEvent && NodeEvent->NodeID == OriginalName)
							{
								NodeEvent->NodeID = NewName;
								NodeEvent->Modify();
							}
						}
					}
				}
			}
		}
	}
}

FSlateColor SGraphNode_DialogBuilderNode::GetBorderBackgroundColor() const
{
	UDialogBuilderEdNode* QuestEdNode = CastChecked<UDialogBuilderEdNode>(GraphNode);
	return QuestEdNode ? QuestEdNode->GetBackgroundColor() : DialogBuilderColors::NodeBorder::HighlightAbortRange;
}

FSlateColor SGraphNode_DialogBuilderNode::GetNodeHighlightColor() const
{
	UDialogBuilderEdNode* QuestEdNode = Cast<UDialogBuilderEdNode>(GraphNode);
	UQuestBuilderNode* QuestNode = QuestEdNode ? Cast<UQuestBuilderNode>(QuestEdNode->NodeInstance) : nullptr;

	if (QuestNode->Quest == nullptr)
		return DialogBuilderColors::NodeBorder::NoHighlight;


	if (QuestNode->Quest->CurrentNode == QuestNode && (GEditor->bIsSimulatingInEditor || GEditor->PlayWorld))
	{
		return DialogBuilderColors::NodeBorder::HighlightCurrentNode;
		
	}
	else
	{
		return DialogBuilderColors::NodeBorder::NoHighlight;
	}
}

FSlateColor SGraphNode_DialogBuilderNode::GetBackgroundColor() const
{
	return DialogBuilderColors::NodeBody::Default;
}

EVisibility SGraphNode_DialogBuilderNode::GetDragOverMarkerVisibility() const
{
	return EVisibility::Visible;
}

EVisibility SGraphNode_DialogBuilderNode::GetDebuggerSearchFailedMarkerVisibility() const
{
	return EVisibility();
}

const FSlateBrush* SGraphNode_DialogBuilderNode::GetNameIcon() const
{
	return FAppStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Icon"));
}

EVisibility SGraphNode_DialogBuilderNode::GetBlueprintIconVisibility() const
{
	UDialogBuilderEdNode* QuestEdNode = CastChecked<UDialogBuilderEdNode>(GraphNode);
	const bool bCanShowIcon = (QuestEdNode != nullptr && QuestEdNode->UsesBlueprint());

	// LOD this out once things get too small
	TSharedPtr<SGraphPanel> MyOwnerPanel = GetOwnerPanel();
	return (bCanShowIcon && (!MyOwnerPanel.IsValid() || MyOwnerPanel->GetCurrentLOD() > EGraphRenderingLOD::LowDetail)) ? EVisibility::Visible : EVisibility::Collapsed;

}

EVisibility SGraphNode_DialogBuilderNode::GetIndexVisibility() const
{
	// always hide the index on the root node
	if (GraphNode->IsA(UDialogBuilderEdNode_Root::StaticClass()))
	{
		return EVisibility::Collapsed;
	}

	UDialogBuilderEdNode* QuestEdNode = CastChecked<UDialogBuilderEdNode>(GraphNode);
	UEdGraphPin* MyInputPin = QuestEdNode->GetInputPin();
	UEdGraphPin* MyParentOutputPin = NULL;
	if (MyInputPin != NULL && MyInputPin->LinkedTo.Num() > 0)
	{
		MyParentOutputPin = MyInputPin->LinkedTo[0];
	}

	
	// LOD this out once things get too small
	TSharedPtr<SGraphPanel> MyOwnerPanel = GetOwnerPanel();

	UDialogBuilderEdGraph* QuestEdGraph = Cast<UDialogBuilderEdGraph>(GraphNode->GetGraph());

	FGraphPanelSelectionSet SelectedNodes;
	if (QuestEdGraph && QuestEdGraph->SEditorGraph)
	{
		SelectedNodes = QuestEdGraph->SEditorGraph->GetSelectedNodes();
	}

	UQuestBuilderNode* FirstSelectedNode = nullptr;
	if (SelectedNodes.Num() > 0)
	{
		// Get the first element of the SelectedNodes set
		UDialogBuilderEdNode* SelectedQuestEdNode = Cast<UDialogBuilderEdNode>(*SelectedNodes.CreateConstIterator());
		FirstSelectedNode = SelectedQuestEdNode ? Cast<UQuestBuilderNode>(SelectedQuestEdNode->NodeInstance) : nullptr;
	}

	UQuestBuilderNode* QuestNode = Cast<UQuestBuilderNode>(QuestEdNode->NodeInstance);

	// Visible if ((we are in PIE || if we have siblings) && (SelectedNodes only one && Matching parent with selected nodes))
	const bool bCanShowIndex = (ShouldShowExecutionIndex() || (MyParentOutputPin && MyParentOutputPin->LinkedTo.Num() > 1)) && (SelectedNodes.Num() == 1 && (FirstSelectedNode && QuestNode) && (QuestNode->ParentNodes.Contains(FirstSelectedNode)));

	return (bCanShowIndex && (!MyOwnerPanel.IsValid() || MyOwnerPanel->GetCurrentLOD() > EGraphRenderingLOD::LowDetail)) ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SGraphNode_DialogBuilderNode::GetIndexText() const
{
	UDialogBuilderEdNode* QuestEdNode = Cast<UDialogBuilderEdNode>(GraphNode);
	UEdGraphPin* MyInputPin = QuestEdNode->GetInputPin();
	UEdGraphPin* MyParentOutputPin = NULL;
	if (MyInputPin != NULL && MyInputPin->LinkedTo.Num() > 0)
	{
		MyParentOutputPin = MyInputPin->LinkedTo[0];
	}

	int32 Index = 0;

	UDialogBuilderEdGraph* QuestEdGraph = Cast<UDialogBuilderEdGraph>(GraphNode->GetGraph());

	FGraphPanelSelectionSet SelectedNodes;
	if (QuestEdGraph && QuestEdGraph->SEditorGraph)
	{
		SelectedNodes = QuestEdGraph->SEditorGraph->GetSelectedNodes();
	}

	UQuestBuilderNode* FirstSelectedNode = nullptr;
	if (SelectedNodes.Num() > 0)
	{
		// Get the first element of the SelectedNodes set
		UDialogBuilderEdNode* SelectedQuestEdNode = Cast<UDialogBuilderEdNode>(*SelectedNodes.CreateConstIterator());
		FirstSelectedNode = SelectedQuestEdNode ? Cast<UQuestBuilderNode>(SelectedQuestEdNode->NodeInstance) : nullptr;
	}

	UQuestBuilderNode* QuestNode = Cast<UQuestBuilderNode>(QuestEdNode->NodeInstance);

	if (QuestNode->ParentNodes.Contains(FirstSelectedNode))
	{
		for (int i = 0; i < FirstSelectedNode->ChildrenNodes.Num(); i++)
		{
			//Select this index if node matches with the selected node childrens
			if (FirstSelectedNode->ChildrenNodes[i] == QuestNode)
			{
				Index = i + 1;//Index start from 1
				break;
			}
		}
	}
	
	
	return FText::AsNumber(Index);
}

FText SGraphNode_DialogBuilderNode::GetIndexTooltipText() const
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

FSlateColor SGraphNode_DialogBuilderNode::GetIndexColor(bool bHovered) const
{
	const bool bHighlightHover = bHovered;

	static const FName HoveredColor("BTEditor.Graph.BTNode.Index.HoveredColor");
	static const FName DefaultColor("BTEditor.Graph.BTNode.Index.Color");

	return bHighlightHover ? FAppStyle::Get().GetSlateColor(HoveredColor) : FAppStyle::Get().GetSlateColor(DefaultColor);
}

void SGraphNode_DialogBuilderNode::OnIndexHoverStateChanged(bool bHovered)
{
}


#undef LOCTEXT_NAMESPACE
