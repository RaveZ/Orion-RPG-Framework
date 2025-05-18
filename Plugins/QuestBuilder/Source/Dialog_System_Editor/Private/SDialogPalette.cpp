// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "SDialogPalette.h"
#include "DialogBuilder_EditorStyle.h"
#include "SlateOptMacros.h"	
#include "TutorialMetaData.h"
#include "SMyDialog.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "EdGraphSchema_DialogBuilder.h"
#include "DialogBuilderEdGraph.h"
#include "DialogBuilderGraph.h"
#include "DialogBuilderNode.h"
#include "Decorator/OrionDecorator.h"
#include "DialogBuilderFunctionLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Event/OrionEvent.h"


#define LOCTEXT_NAMESPACE "DialogPaletteItem"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SDialogPaletteItem::Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData, TWeakPtr<FDialogBuilderEditor> InDialogEditor)
{
	Construct(InArgs, InCreateData, InDialogEditor.Pin()->GetDialogBuilderGraph(), InDialogEditor);

}

void SDialogPaletteItem::Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData, UDialogBuilderGraph* InDialogSystemGraph)
{
	Construct(InArgs, InCreateData, InDialogSystemGraph, TWeakPtr<FDialogBuilderEditor>());
}

void SDialogPaletteItem::Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData, UDialogBuilderGraph* InDialogSystemGraph, TWeakPtr<FDialogBuilderEditor> InDialogEditor)
{
	check(InCreateData->Action.IsValid());
	check(InDialogSystemGraph);

	DialogBuilderGraph = InDialogSystemGraph;

	bShowClassInTooltip = InArgs._ShowClassInTooltip;

	TSharedPtr<FEdGraphSchemaAction> GraphAction = InCreateData->Action;
	ActionPtr = InCreateData->Action;
	DialogEditorPtr = InDialogEditor;

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
	if (DialogEditorPtr.IsValid())
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
	const UEdGraphSchema_DialogBuilder* DialogSchema = GetDefault<UEdGraphSchema_DialogBuilder>();
	if (MenuDescriptionCache.IsOutOfDate(DialogSchema))
	{
		TSharedPtr< FEdGraphSchemaAction > GraphAction = ActionPtr.Pin();
		FAssetSchemaAction_DialogBuilderGraph* DialogGraphAction = (FAssetSchemaAction_DialogBuilderGraph*)GraphAction.Get();
		if (UDialogBuilderEdGraph* DialogEdGraph = Cast<UDialogBuilderEdGraph>(DialogGraphAction->EdGraph))
		{
			MenuDescriptionCache.SetCachedText(FText::FromName(DialogEdGraph->Dialog->ID), DialogSchema);
		}
	}

	return MenuDescriptionCache;
}

bool SDialogPaletteItem::OnNameTextVerifyChanged(const FText& InNewText, FText& OutErrorMessage)
{
	FString FinalString = InNewText.ToString().TrimStartAndEnd();

	FName OriginalName;

	FAssetSchemaAction_DialogBuilderGraph* DialogGraphAction = (FAssetSchemaAction_DialogBuilderGraph*)ActionPtr.Pin().Get();
	if (UDialogBuilderEdGraph* DialogEdGraph = Cast<UDialogBuilderEdGraph>(DialogGraphAction->EdGraph))
	{
		OriginalName = DialogEdGraph->Dialog->ID;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> DialogGraphDataArray;
	AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UDialogBuilderGraph::StaticClass()), DialogGraphDataArray);

	for (const FAssetData& AssetData : DialogGraphDataArray)
	{
		UDialogBuilderGraph* DialogGraph = Cast<UDialogBuilderGraph>(AssetData.GetAsset());
		if (DialogGraph)
		{
			for (auto& Dialog : DialogGraph->DialogList)
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

  FAssetSchemaAction_DialogBuilderGraph* DialogGraphAction = (FAssetSchemaAction_DialogBuilderGraph*)ActionPtr.Pin().Get();  
  if (UDialogBuilderEdGraph* DialogEdGraph = Cast<UDialogBuilderEdGraph>(DialogGraphAction->EdGraph))  
  {
      OriginalName = DialogEdGraph->Dialog->ID;  
      DialogEdGraph->Dialog->ID = NewName;  
      DialogEdGraph->Rename(*(NewName.ToString()), DialogBuilderGraph, REN_DoNotDirty | REN_ForceNoResetLoaders);  


      if (!DialogEdGraph->Dialog->DialogName.IsFromStringTable())
      {
          if (DialogEdGraph->Dialog->DialogName.IsEmpty() || DialogEdGraph->Dialog->DialogName.EqualTo(FText::FromName(OriginalName)))
          {
              DialogEdGraph->Dialog->DialogName = FText::FromName(NewName);
          }
      }

      FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

      TArray<FAssetData> DialogGraphDataArray;
      AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UDialogBuilderGraph::StaticClass()), DialogGraphDataArray);

      /*for (const FAssetData& AssetData : DialogGraphDataArray)
      {
          UDialogBuilderGraph* DialogGraph = Cast<UDialogBuilderGraph>(AssetData.GetAsset());
          if (DialogGraph)
          {
              for (auto& Dialog : DialogGraph->DialogList)
              {
                  for (auto& DialogCondition : Dialog->Decorators)
                  {
                      if (DialogCondition && DialogCondition->DialogID == OriginalName)
                      {
                          DialogCondition->DialogID = NewName;
                          DialogCondition->Modify();
                      }
                  }
                  for (auto& Node : Dialog->AllNodes)
                  {
                      for (auto& NodeCondition : Node->Decorators)
                      {
                          if (NodeCondition && NodeCondition->DialogID == OriginalName)
                          {
                              NodeCondition->DialogID = NewName;
                              NodeCondition->Modify();
                          }
                      }
                      for (auto& NodeEvent : Node->Events)
                      {
                          if (NodeEvent && NodeEvent->DialogID == OriginalName)
                          {
                              NodeEvent->DialogID = NewName;
                              NodeEvent->Modify();
                          }
                      }
                  }
              }
          }
      }*/
  }  
  DialogEditorPtr.Pin()->GetMyDialogWidget()->Refresh();  

  DialogEditorPtr.Pin()->GetDialogBuilderGraph()->Modify();  
}

TSharedPtr<SToolTip> SDialogPaletteItem::ConstructToolTipWidget() const
{

	return TSharedPtr<SToolTip>();
}

FText SDialogPaletteItem::GetToolTipText() const
{
	return FText::FromString(TEXT(""));
}

void SDialogPalette::Construct(const FArguments& InArgs, TWeakPtr<FDialogBuilderEditor> InDialogEditor)
{
}

void SDialogPalette::OnSplitterResized() const
{
}

#undef LOCTEXT_NAMESPACE