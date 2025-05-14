// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "SQuestPalette.h"
#include "QuestBuilder_EditorStyle.h"
#include "SlateOptMacros.h"	
#include "TutorialMetaData.h"
#include "SMyQuest.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "EdGraphSchema_QuestBuilder.h"
#include "QuestBuilderEdGraph.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"
#include "Decorator/OrionDecorator.h"
#include "QuestBuilderFunctionLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Event/OrionEvent.h"


#define LOCTEXT_NAMESPACE "QuestPaletteItem"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SQuestPaletteItem::Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData, TWeakPtr<FQuestBuilderEditor> InQuestEditor)
{
	Construct(InArgs, InCreateData, InQuestEditor.Pin()->GetQuestBuilderGraph(), InQuestEditor);

}

void SQuestPaletteItem::Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData, UQuestBuilderGraph* InQuestSystemGraph)
{
	Construct(InArgs, InCreateData, InQuestSystemGraph, TWeakPtr<FQuestBuilderEditor>());
}

void SQuestPaletteItem::Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData, UQuestBuilderGraph* InQuestSystemGraph, TWeakPtr<FQuestBuilderEditor> InQuestEditor)
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



void SQuestPaletteItem::OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	if (QuestEditorPtr.IsValid())
	{
		SGraphPaletteItem::OnDragEnter(MyGeometry, DragDropEvent);
	}
}

TSharedRef<SWidget> SQuestPaletteItem::CreateTextSlotWidget(FCreateWidgetForActionData* const InCreateData, TAttribute<bool> bIsReadOnly)
{
	FName const ActionTypeId = InCreateData->Action->GetTypeId();

	FOnVerifyTextChanged OnVerifyTextChanged;
	FOnTextCommitted     OnTextCommitted;

	// default to our own rename methods
	OnVerifyTextChanged.BindSP(this, &SQuestPaletteItem::OnNameTextVerifyChanged);
	OnTextCommitted.BindSP(this, &SQuestPaletteItem::OnNameTextCommitted);

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
				.Text(this, &SQuestPaletteItem::GetDisplayText)
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

FText SQuestPaletteItem::GetDisplayText() const
{
	const UEdGraphSchema_QuestBuilder* QuestSchema = GetDefault<UEdGraphSchema_QuestBuilder>();
	if (MenuDescriptionCache.IsOutOfDate(QuestSchema))
	{
		TSharedPtr< FEdGraphSchemaAction > GraphAction = ActionPtr.Pin();
		FAssetSchemaAction_QuestSystemGraph* QuestGraphAction = (FAssetSchemaAction_QuestSystemGraph*)GraphAction.Get();
		if (UQuestBuilderEdGraph* QuestEdGraph = Cast<UQuestBuilderEdGraph>(QuestGraphAction->EdGraph))
		{
			MenuDescriptionCache.SetCachedText(FText::FromName(QuestEdGraph->Quest->ID), QuestSchema);
		}
	}

	return MenuDescriptionCache;
}

bool SQuestPaletteItem::OnNameTextVerifyChanged(const FText& InNewText, FText& OutErrorMessage)
{
	FString FinalString = InNewText.ToString().TrimStartAndEnd();

	FName OriginalName;

	FAssetSchemaAction_QuestSystemGraph* QuestGraphAction = (FAssetSchemaAction_QuestSystemGraph*)ActionPtr.Pin().Get();
	if (UQuestBuilderEdGraph* QuestEdGraph = Cast<UQuestBuilderEdGraph>(QuestGraphAction->EdGraph))
	{
		OriginalName = QuestEdGraph->Quest->ID;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> QuestGraphDataArray;
	AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UQuestBuilderGraph::StaticClass()), QuestGraphDataArray);

	for (const FAssetData& AssetData : QuestGraphDataArray)
	{
		UQuestBuilderGraph* QuestGraph = Cast<UQuestBuilderGraph>(AssetData.GetAsset());
		if (QuestGraph)
		{
			for (auto& Quest : QuestGraph->QuestList)
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
				if (Quest->ID == FName(FinalString))
				{
					OutErrorMessage = LOCTEXT("RenameFailed_NotValid", "This ID is already used in another Quest.");
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

void SQuestPaletteItem::OnNameTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit)  
{  
  const FString NewNameString = NewText.ToString().TrimStartAndEnd();  
  const FName NewName = *NewNameString;  
  FName OriginalName;  

  FAssetSchemaAction_QuestSystemGraph* QuestGraphAction = (FAssetSchemaAction_QuestSystemGraph*)ActionPtr.Pin().Get();  
  if (UQuestBuilderEdGraph* QuestEdGraph = Cast<UQuestBuilderEdGraph>(QuestGraphAction->EdGraph))  
  {
      OriginalName = QuestEdGraph->Quest->ID;  
      QuestEdGraph->Quest->ID = NewName;  
      QuestEdGraph->Rename(*(NewName.ToString()), QuestBuilderGraph, REN_DoNotDirty | REN_ForceNoResetLoaders);  


      if (!QuestEdGraph->Quest->QuestName.IsFromStringTable())
      {
          if (QuestEdGraph->Quest->QuestName.IsEmpty() || QuestEdGraph->Quest->QuestName.EqualTo(FText::FromName(OriginalName)))
          {
              QuestEdGraph->Quest->QuestName = FText::FromName(NewName);
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
              for (auto& Quest : QuestGraph->QuestList)
              {
                  for (auto& QuestCondition : Quest->Conditions)
                  {
                      if (QuestCondition && QuestCondition->QuestID == OriginalName)
                      {
                          QuestCondition->QuestID = NewName;
                          QuestCondition->Modify();
                      }
                  }
                  for (auto& Node : Quest->AllNodes)
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

TSharedPtr<SToolTip> SQuestPaletteItem::ConstructToolTipWidget() const
{

	return TSharedPtr<SToolTip>();
}

FText SQuestPaletteItem::GetToolTipText() const
{
	return FText::FromString(TEXT(""));
}

void SQuestPalette::Construct(const FArguments& InArgs, TWeakPtr<FQuestBuilderEditor> InQuestEditor)
{
}

void SQuestPalette::OnSplitterResized() const
{
}

#undef LOCTEXT_NAMESPACE