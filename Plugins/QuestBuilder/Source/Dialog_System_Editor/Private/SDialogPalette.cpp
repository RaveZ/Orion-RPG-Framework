// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "SDialogPalette.h"
#include "DialogBuilder_EditorStyle.h"
#include "SlateOptMacros.h"	
#include "TutorialMetaData.h"
#include "SMyDialog.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "EdGraphSchema_DialogBuilder.h"
#include "DialogBuilderEdGraph.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"
#include "Decorator/OrionDecorator.h"
#include "QuestBuilderFunctionLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Event/OrionEvent.h"


#define LOCTEXT_NAMESPACE "QuestPaletteItem"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SDialogPaletteItem::Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData, TWeakPtr<FDialogBuilderEditor> InQuestEditor)
{
	Construct(InArgs, InCreateData, InQuestEditor.Pin()->GetQuestBuilderGraph(), InQuestEditor);

}

void SDialogPaletteItem::Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData, UQuestBuilderGraph* InQuestSystemGraph)
{
	Construct(InArgs, InCreateData, InQuestSystemGraph, TWeakPtr<FDialogBuilderEditor>());
}

void SDialogPaletteItem::Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData, UQuestBuilderGraph* InQuestSystemGraph, TWeakPtr<FDialogBuilderEditor> InQuestEditor)
{
	check(InCreateData->Action.IsValid());
	check(InQuestSystemGraph);

	QuestBuilderGraph = InQuestSystemGraph;

	bShowClassInTooltip = InArgs._ShowClassInTooltip;

	TSharedPtr<FEdGraphSchemaAction> GraphAction = InCreateData->Action;
	ActionPtr = InCreateData->Action;
	QuestEditorPtr = InQuestEditor;

	// construct the icon widget
	FSlateBrush const* IconBrush = FAppStyle::GetBrush(TEXT("GraphEditor.EventGraph_16x"));
	FSlateBrush const* SecondaryBrush = FAppStyle::GetBrush(TEXT("NoBrush"));
	FSlateColor        IconColor = FSlateColor::UseForeground();
	FSlateColor        SecondaryIconColor = FSlateColor::UseForeground();
	FText			   IconToolTip = GraphAction->GetTooltipDescription();
	FString			   IconDocLink, IconDocExcerpt;
	TSharedRef<SWidget> IconWidget = CreateIconWidget(IconToolTip, IconBrush, IconColor);
	
	// Setup a meta tag for this node
	FTutorialMetaData TagMeta("PaletteItem");
	if (ActionPtr.IsValid())
	{
		TagMeta.Tag = *FString::Printf(TEXT("PaletteItem,%s,%d"), *GraphAction->GetMenuDescription().ToString(), GraphAction->GetSectionID());
		TagMeta.FriendlyName = GraphAction->GetMenuDescription().ToString();
	}

	// construct the text widget
	bool bIsReadOnly = false;
	TSharedRef<SWidget> NameSlotWidget = CreateTextSlotWidget(InCreateData, bIsReadOnly);

	// Create the widget with an icon
	TSharedRef<SHorizontalBox> ActionBox = SNew(SHorizontalBox)
		.AddMetaData<FTutorialMetaData>(TagMeta);


	ActionBox.Get().AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			IconWidget
		];


	ActionBox.Get().AddSlot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		.Padding(/* horizontal */ 3.0f, /* vertical */ 3.0f)
		[
			NameSlotWidget
		];

	// Now, create the actual widget
	ChildSlot
		[
			ActionBox
		];

}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION



void SDialogPaletteItem::OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	if (QuestEditorPtr.IsValid())
	{
		SGraphPaletteItem::OnDragEnter(MyGeometry, DragDropEvent);
	}
}

TSharedRef<SWidget> SDialogPaletteItem::CreateTextSlotWidget(FCreateWidgetForActionData* const InCreateData, TAttribute<bool> bIsReadOnly)
{
	FName const ActionTypeId = InCreateData->Action->GetTypeId();

	FOnVerifyTextChanged OnVerifyTextChanged;
	FOnTextCommitted     OnTextCommitted;

	// default to our own rename methods
	OnVerifyTextChanged.BindSP(this, &SDialogPaletteItem::OnNameTextVerifyChanged);
	OnTextCommitted.BindSP(this, &SDialogPaletteItem::OnNameTextCommitted);

	// Copy the mouse delegate binding if we want it
	if (InCreateData->bHandleMouseButtonDown)
	{
		MouseButtonDownDelegate = InCreateData->MouseButtonDownDelegate;
	}

	TSharedPtr<SToolTip> ToolTipWidget = ConstructToolTipWidget();

	TSharedPtr<SOverlay> DisplayWidget;
	TSharedPtr<SInlineEditableTextBlock> EditableTextElement;
	SAssignNew(DisplayWidget, SOverlay)
		+ SOverlay::Slot()
		[
			SAssignNew(EditableTextElement, SInlineEditableTextBlock)
				.Text(this, &SDialogPaletteItem::GetDisplayText)
				.HighlightText(InCreateData->HighlightText)
				.ToolTip(ToolTipWidget)
				.OnVerifyTextChanged(OnVerifyTextChanged)
				.OnTextCommitted(OnTextCommitted)
				.IsSelected(InCreateData->IsRowSelectedDelegate)
				.IsReadOnly(false)
		];
	InlineRenameWidget = EditableTextElement.ToSharedRef();

	InCreateData->OnRenameRequest->BindSP(InlineRenameWidget.Get(), &SInlineEditableTextBlock::EnterEditingMode);

	return DisplayWidget.ToSharedRef();
}

FText SDialogPaletteItem::GetDisplayText() const
{
	const UEdGraphSchema_DialogBuilder* QuestSchema = GetDefault<UEdGraphSchema_DialogBuilder>();
	if (MenuDescriptionCache.IsOutOfDate(QuestSchema))
	{
		TSharedPtr< FEdGraphSchemaAction > GraphAction = ActionPtr.Pin();
		FAssetSchemaAction_DialogBuilderGraph* QuestGraphAction = (FAssetSchemaAction_DialogBuilderGraph*)GraphAction.Get();
		if (UDialogBuilderEdGraph* QuestEdGraph = Cast<UDialogBuilderEdGraph>(QuestGraphAction->EdGraph))
		{
			MenuDescriptionCache.SetCachedText(FText::FromName(QuestEdGraph->Dialog->ID), QuestSchema);
		}
	}

	return MenuDescriptionCache;
}

bool SDialogPaletteItem::OnNameTextVerifyChanged(const FText& InNewText, FText& OutErrorMessage)
{
	FString FinalString = InNewText.ToString().TrimStartAndEnd();

	FName OriginalName;

	FAssetSchemaAction_DialogBuilderGraph* QuestGraphAction = (FAssetSchemaAction_DialogBuilderGraph*)ActionPtr.Pin().Get();
	if (UDialogBuilderEdGraph* QuestEdGraph = Cast<UDialogBuilderEdGraph>(QuestGraphAction->EdGraph))
	{
		OriginalName = QuestEdGraph->Dialog->ID;
	}

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
				if (FName(FinalString) == OriginalName)
				{
					return true;
				}
				if (FName(FinalString).IsNone())
				{
					OutErrorMessage = LOCTEXT("RenameFailed_NotValid", "Empty ID is Not Valid.");
					return false;
				}
				if (Dialog->ID == FName(FinalString))
				{
					OutErrorMessage = LOCTEXT("RenameFailed_NotValid", "This ID is already used in another Dialog.");
					return false;
				}
			}
		}
	}

	UStruct* ValidationScope = nullptr;

	const UEdGraphSchema* Schema = nullptr;

	return true;
}

#include "GameFramework/Actor.h"
#include "Logging/LogMacros.h"

void SDialogPaletteItem::OnNameTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit)  
{  
  const FString NewNameString = NewText.ToString().TrimStartAndEnd();  
  const FName NewName = *NewNameString;  
  FName OriginalName;  

  FAssetSchemaAction_DialogBuilderGraph* QuestGraphAction = (FAssetSchemaAction_DialogBuilderGraph*)ActionPtr.Pin().Get();  
  if (UDialogBuilderEdGraph* QuestEdGraph = Cast<UDialogBuilderEdGraph>(QuestGraphAction->EdGraph))  
  {
      OriginalName = QuestEdGraph->Dialog->ID;  
      QuestEdGraph->Dialog->ID = NewName;  
      QuestEdGraph->Rename(*(NewName.ToString()), QuestBuilderGraph, REN_DoNotDirty | REN_ForceNoResetLoaders);  


      if (!QuestEdGraph->Dialog->QuestName.IsFromStringTable())
      {
          if (QuestEdGraph->Dialog->QuestName.IsEmpty() || QuestEdGraph->Dialog->QuestName.EqualTo(FText::FromName(OriginalName)))
          {
              QuestEdGraph->Dialog->QuestName = FText::FromName(NewName);
          }
      }

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
                      if (QuestCondition && QuestCondition->QuestID == OriginalName)
                      {
                          QuestCondition->QuestID = NewName;
                          QuestCondition->Modify();
                      }
                  }
                  for (auto& Node : Dialog->AllNodes)
                  {
                      for (auto& NodeCondition : Node->Conditions)
                      {
                          if (NodeCondition && NodeCondition->QuestID == OriginalName)
                          {
                              NodeCondition->QuestID = NewName;
                              NodeCondition->Modify();
                          }
                      }
                      for (auto& NodeEvent : Node->Events)
                      {
                          if (NodeEvent && NodeEvent->QuestID == OriginalName)
                          {
                              NodeEvent->QuestID = NewName;
                              NodeEvent->Modify();
                          }
                      }
                  }
              }
          }
      }
  }  
  QuestEditorPtr.Pin()->GetMyQuestWidget()->Refresh();  

  QuestEditorPtr.Pin()->GetQuestBuilderGraph()->Modify();  
}

TSharedPtr<SToolTip> SDialogPaletteItem::ConstructToolTipWidget() const
{

	return TSharedPtr<SToolTip>();
}

FText SDialogPaletteItem::GetToolTipText() const
{
	return FText::FromString(TEXT(""));
}

void SQuestPalette::Construct(const FArguments& InArgs, TWeakPtr<FDialogBuilderEditor> InQuestEditor)
{
}

void SQuestPalette::OnSplitterResized() const
{
}

#undef LOCTEXT_NAMESPACE