// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "SGraphNode_QuestSystemNode.h"
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
#include "Colors_QuestSystem.h"
#include "GraphEditorSettings.h"
#include "SCommentBubble.h"
#include "SLevelOfDetailBranchNode.h"
#include "QuestSystemGraphNode.h"
#include "QuestSystemGraphNode_Objective.h"
#include "Setting_QuestEditor.h"	
#include "Fonts/SlateFontInfo.h"
#include "QuestNodeWidget.h"
#include "Condition/QuestCondition.h"
#include "Event/QuestEvent.h"

#define LOCTEXT_NAMESPACE "EdNode_GenericGraph"

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
		return bIsDiffHighlighted ? QuestSystemColors::Pin::Diff :
			IsHovered() ? QuestSystemColors::Pin::Hover : QuestSystemColors::Pin::Default;
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


void SGraphNode_QuestSystemNode::Construct(const FArguments& InArgs, UEdNode_QuestSystemNode* InNode)
{
	GraphNode = InNode;
	UpdateGraphNode();
	InNode->SGraphNode = this;
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SGraphNode_QuestSystemNode::UpdateGraphNode()
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

	UEdNode_QuestSystemNode* QuestEditorNode = Cast<UEdNode_QuestSystemNode>(GraphNode);
	UQuestSystemGraphNode* QuestSystemNode = QuestEditorNode ? QuestEditorNode->QuestSystemGraphNode : nullptr;

	UQuestNodeWidget* QuestNodeWidgetInstance = nullptr;
	UWorld* World = GEditor->GetEditorWorldContext().World();

	// In a function where you want to get the reference to the UUserWidget asset
	// Load the UUserWidget asset using SoftObjectPtr::LoadSynchronous
	if (const UQuestEditorSettings* QuestSettings = GetDefault<UQuestEditorSettings>())
	{
		UClass* WidgetClass = QuestSettings->DefaultQuestNodeWidgetClass.LoadSynchronous();

		// Spawn an instance of the UUserWidget class
		QuestNodeWidgetInstance = CreateWidget<UQuestNodeWidget>(World, WidgetClass);
	}


	if (QuestNodeWidgetInstance && QuestSystemNode)
	{
		if (QuestSystemNode->QuestSystem)
		{
			QuestNodeWidgetInstance->InitializeFromNode(QuestSystemNode, QuestSystemNode->QuestSystem);
			QuestNodeWidgetRef = QuestNodeWidgetInstance->TakeWidget();

			if (QuestNodeWidgetInstance->RightPinBox && QuestNodeWidgetInstance->LeftPinBox)
			{
				//Tell the C++ code to autopopulate our UMG blueprint widgets pin boxes! 
				RightNodeBox = StaticCastSharedRef<SVerticalBox>(QuestNodeWidgetInstance->RightPinBox->TakeWidget());
				LeftNodeBox = StaticCastSharedRef<SVerticalBox>(QuestNodeWidgetInstance->LeftPinBox->TakeWidget());
			}
			else
			{
				//Didnt have pin boxes, clear the Quest node ref to force node to use old style 
				QuestNodeWidgetRef.Reset();
			}
		}
		
	}
	
	//UMG Based Nodes
	/*if (QuestNodeWidgetRef.IsValid())
	{

		this->GetOrAddSlot(ENodeZone::Center)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SOverlay)
					+ SOverlay::Slot()
					[
						QuestNodeWidgetRef.ToSharedRef()
					]
			];
	}
	else //Old Slate Style Nodes
	{*/
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
					.BorderBackgroundColor(this, &SGraphNode_QuestSystemNode::GetNodeHighlightColor)
					[
						SNew(SBorder)
							.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
							.Padding(0.0f)
							.BorderBackgroundColor(this, &SGraphNode_QuestSystemNode::GetBorderBackgroundColor)
							//.OnMouseButtonDown(this, &SGraphNode_QuestSystemNode::OnMouseDown)
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
																					.OnVerifyTextChanged(this, &SGraphNode_QuestSystemNode::OnVerifyNameTextChanged)
																					.OnTextCommitted(this, &SGraphNode_QuestSystemNode::OnNameTextCommited)
																					.IsReadOnly(this, &SGraphNode_QuestSystemNode::IsNameReadOnly)
																					.IsSelected(this, &SGraphNode_QuestSystemNode::IsSelectedExclusively)
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
																			.BackgroundColor(this, &SGraphNode_QuestSystemNode::GetErrorColor)
																			.ToolTipText(this, &SGraphNode_QuestSystemNode::GetErrorMsgToolTip)
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
																	.Visibility(this, &SGraphNode_QuestSystemNode::GetTaskVisibility)
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
																	.Visibility(this, &SGraphNode_QuestSystemNode::GetDescriptionVisibility)
																	.Text(this, &SGraphNode_QuestSystemNode::GetDescription)
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
																	.Visibility(this, &SGraphNode_QuestSystemNode::GetEventsVisibility)
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
																	.Visibility(this, &SGraphNode_QuestSystemNode::GetEventsVisibility)
																	.Text(this, &SGraphNode_QuestSystemNode::GetEventsText)
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
																	.Visibility(this, &SGraphNode_QuestSystemNode::GetConditionsVisibility)
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
																	.Visibility(this, &SGraphNode_QuestSystemNode::GetConditionsVisibility)
																	.Text(this, &SGraphNode_QuestSystemNode::GetConditionsText)
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
											.Visibility(this, &SGraphNode_QuestSystemNode::GetBlueprintIconVisibility)
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

void SGraphNode_QuestSystemNode::CreatePinWidgets()
{
	UEdNode_QuestSystemNode* StateNode = CastChecked<UEdNode_QuestSystemNode>(GraphNode);

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

void SGraphNode_QuestSystemNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
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

bool SGraphNode_QuestSystemNode::IsNameReadOnly() const
{
	UEdNode_QuestSystemNode* EdNode_Node = Cast<UEdNode_QuestSystemNode>(GraphNode);
	check(EdNode_Node != nullptr);

	UQuestSystemGraph* QuestSystemGraph = EdNode_Node->QuestSystemGraphNode->QuestGraph;
	check(QuestSystemGraph != nullptr);

	return (!QuestSystemGraph->bCanRenameNode || !EdNode_Node->QuestSystemGraphNode->IsNameEditable()) || SGraphNode::IsNameReadOnly();
}
FText SGraphNode_QuestSystemNode::GetDescription() const
{
	UEdNode_QuestSystemNode* QuestEditorNode = CastChecked<UEdNode_QuestSystemNode>(GraphNode);
	return QuestEditorNode ? QuestEditorNode->GetDescription() : FText::GetEmpty();
}
EVisibility SGraphNode_QuestSystemNode::GetDescriptionVisibility() const
{
	// LOD this out once things get too small
	TSharedPtr<SGraphPanel> MyOwnerPanel = GetOwnerPanel();
	return (!MyOwnerPanel.IsValid() || MyOwnerPanel->GetCurrentLOD() > EGraphRenderingLOD::LowDetail) ? EVisibility::Visible : EVisibility::Collapsed;

}
EVisibility SGraphNode_QuestSystemNode::GetTaskVisibility() const
{
	UEdNode_QuestSystemNode* QuestEditorNode = CastChecked<UEdNode_QuestSystemNode>(GraphNode);
	UQuestSystemGraphNode* QuestSystemNode = QuestEditorNode ? QuestEditorNode->QuestSystemGraphNode : nullptr;

	if (UQuestSystemGraphNode_Objective* QuestSystemNodeObjective = Cast<UQuestSystemGraphNode_Objective>(QuestSystemNode))
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}
FText SGraphNode_QuestSystemNode::GetEventsText() const
{
	UEdNode_QuestSystemNode* QuestEditorNode = CastChecked<UEdNode_QuestSystemNode>(GraphNode);
	return QuestEditorNode ? QuestEditorNode->GetEventsText() : FText::GetEmpty();
}
EVisibility SGraphNode_QuestSystemNode::GetEventsVisibility() const
{
	UEdNode_QuestSystemNode* QuestEditorNode = CastChecked<UEdNode_QuestSystemNode>(GraphNode);
	UQuestSystemGraphNode* QuestSystemNode = QuestEditorNode ? QuestEditorNode->QuestSystemGraphNode : nullptr;
	if (QuestSystemNode && !QuestSystemNode->Events.IsEmpty())
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}
FText SGraphNode_QuestSystemNode::GetConditionsText() const
{
	UEdNode_QuestSystemNode* QuestEditorNode = CastChecked<UEdNode_QuestSystemNode>(GraphNode);
	return QuestEditorNode ? QuestEditorNode->GetConditionsText() : FText::GetEmpty();
}
EVisibility SGraphNode_QuestSystemNode::GetConditionsVisibility() const
{
	UEdNode_QuestSystemNode* QuestEditorNode = CastChecked<UEdNode_QuestSystemNode>(GraphNode);
	UQuestSystemGraphNode* QuestSystemNode = QuestEditorNode ? QuestEditorNode->QuestSystemGraphNode : nullptr;
	if (QuestSystemNode && !QuestSystemNode->Conditions.IsEmpty())
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}
bool SGraphNode_QuestSystemNode::OnVerifyNameTextChanged(const FText& InText, FText& OutErrorMessage)
{
	FString TextAsString = InText.ToString();

	FName OriginalName;

	bool bValid(true);

	if ((GetEditableNodeTitle() != InText.ToString()) && OnVerifyTextCommit.IsBound())
	{
		OutErrorMessage = LOCTEXT("RenameFailed_NotValid", "Name Not Valid.");
		bValid = OnVerifyTextCommit.Execute(InText, GraphNode, OutErrorMessage);
	}

	if (UEdNode_QuestSystemNode* MyNode = Cast<UEdNode_QuestSystemNode>(GraphNode))
	{
		OriginalName = MyNode->QuestSystemGraphNode->ID;
	}


	for (TObjectIterator<UQuestSystemGraphNode> It; It; ++It)
	{
		if (UQuestSystemGraphNode* QuestSystem = Cast<UQuestSystemGraphNode>(*It))
		{
			if (FName(InText.ToString()) == OriginalName)
			{
				return true;
			}
			if (QuestSystem->ID == FName(InText.ToString()))
			{
				OutErrorMessage = LOCTEXT("RenameFailed_NotValid", "This ID is already used in another Node.");
				return false;
			}
		}
	}


	return bValid;
}
FText SGraphNode_QuestSystemNode::GetPinTooltip(UEdGraphPin* GraphPinObj) const
{
	return FText();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SGraphNode_QuestSystemNode::OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo)
{
	SGraphNode::OnNameTextCommited(InText, CommitInfo);

	const FString NewNameString = InText.ToString();
	const FName NewName = *NewNameString;
	FName OriginalName;

	UEdNode_QuestSystemNode* MyNode = CastChecked<UEdNode_QuestSystemNode>(GraphNode);
	UQuestSystemGraphNode* QuestSystemNode = MyNode ? MyNode->QuestSystemGraphNode : nullptr;

	if (MyNode != nullptr && MyNode->QuestSystemGraphNode != nullptr)
	{
		UEdGraph_QuestSystemGraph* QuestEdGraph = MyNode->GetQuestSystemGraphEdGraph();
		OriginalName = QuestSystemNode->ID;
		
		//Finalize Node and Update GraphNode
		const FScopedTransaction Transaction(LOCTEXT("QuestSystemEditorRenameNode", "Quest System Editor: Rename Node"));
		MyNode->Modify();
		MyNode->QuestSystemGraphNode->Modify();
		MyNode->QuestSystemGraphNode->SetNodeTitle(InText);
		UpdateGraphNode();

		// Update the name of the node in the QuestSystemGraph
		for (TObjectIterator<UQuestCondition> It; It; ++It)
		{
			if (UQuestCondition* QuestCondition = Cast<UQuestCondition>(*It))
			{
				if (QuestCondition->NodeID == OriginalName)
				{
					QuestCondition->NodeID = NewName;
				}
			}
		}
		for (TObjectIterator<UQuestEvent> It; It; ++It)
		{
			if (UQuestEvent* QuestEvent = Cast<UQuestEvent>(*It))
			{
				if (QuestEvent->NodeID == OriginalName)
				{
					QuestEvent->NodeID = NewName;
				}
			}
		}
	}
}

FSlateColor SGraphNode_QuestSystemNode::GetBorderBackgroundColor() const
{
	UEdNode_QuestSystemNode* MyNode = CastChecked<UEdNode_QuestSystemNode>(GraphNode);
	return MyNode ? MyNode->GetBackgroundColor() : QuestSystemColors::NodeBorder::HighlightAbortRange0;
}

FSlateColor SGraphNode_QuestSystemNode::GetNodeHighlightColor() const
{
	UEdNode_QuestSystemNode* MyNode = CastChecked<UEdNode_QuestSystemNode>(GraphNode);
	if (MyNode->QuestSystemGraphNode->QuestSystem == nullptr)
		return QuestSystemColors::NodeBorder::NoHighlight;


	if (MyNode->QuestSystemGraphNode->QuestSystem->CurrentNode == MyNode->QuestSystemGraphNode && (GEditor->bIsSimulatingInEditor || GEditor->PlayWorld))
	{
		return QuestSystemColors::NodeBorder::HighlightCurrentNode;
		
	}
	else
	{
		return QuestSystemColors::NodeBorder::NoHighlight;
	}
}

FSlateColor SGraphNode_QuestSystemNode::GetBackgroundColor() const
{
	return QuestSystemColors::NodeBody::Default;
}

EVisibility SGraphNode_QuestSystemNode::GetDragOverMarkerVisibility() const
{
	return EVisibility::Visible;
}

EVisibility SGraphNode_QuestSystemNode::GetDebuggerSearchFailedMarkerVisibility() const
{
	return EVisibility();
}

const FSlateBrush* SGraphNode_QuestSystemNode::GetNameIcon() const
{
	return FAppStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Icon"));
}

EVisibility SGraphNode_QuestSystemNode::GetBlueprintIconVisibility() const
{
	UEdNode_QuestSystemNode* MyNode = CastChecked<UEdNode_QuestSystemNode>(GraphNode);
	const bool bCanShowIcon = (MyNode != nullptr && MyNode->UsesBlueprint());

	// LOD this out once things get too small
	TSharedPtr<SGraphPanel> MyOwnerPanel = GetOwnerPanel();
	return (bCanShowIcon && (!MyOwnerPanel.IsValid() || MyOwnerPanel->GetCurrentLOD() > EGraphRenderingLOD::LowDetail)) ? EVisibility::Visible : EVisibility::Collapsed;

}

#undef LOCTEXT_NAMESPACE
