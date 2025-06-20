// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "SInventoryPalette.h"
#include "OrionInventory_EditorStyle.h"
#include "OrionInventoryEditor.h"
#include "SlateOptMacros.h"	
#include "TutorialMetaData.h"
#include "SMyInventory.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "EdGraphSchema_OrionInventory.h"
#include "OrionInventoryEdGraph.h"
#include "Inventory/OrionInventoryGraph.h"
#include "Decorator/OrionDecorator.h"
#include "DialogBuilderFunctionLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Event/OrionEvent.h"


#define LOCTEXT_NAMESPACE "InventoryPaletteItem"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SInventoryPaletteItem::Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData, TWeakPtr<FOrionInventoryEditor> InInventoryEditor)
{
	Construct(InArgs, InCreateData, InInventoryEditor.Pin()->GetOrionInventoryGraph(), InInventoryEditor);

}

void SInventoryPaletteItem::Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData, UOrionInventoryGraph* InInventorySystemGraph)
{
	Construct(InArgs, InCreateData, InInventorySystemGraph, TWeakPtr<FOrionInventoryEditor>());
}

void SInventoryPaletteItem::Construct(const FArguments& InArgs, FCreateWidgetForActionData* const InCreateData, UOrionInventoryGraph* InInventorySystemGraph, TWeakPtr<FOrionInventoryEditor> InInventoryEditor)
{
	check(InCreateData->Action.IsValid());
	check(InInventorySystemGraph);

	OrionInventoryGraph = InInventorySystemGraph;

	bShowClassInTooltip = InArgs._ShowClassInTooltip;

	TSharedPtr<FEdGraphSchemaAction> GraphAction = InCreateData->Action;
	ActionPtr = InCreateData->Action;
	InventoryEditorPtr = InInventoryEditor;

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



void SInventoryPaletteItem::OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	if (InventoryEditorPtr.IsValid())
	{
		SGraphPaletteItem::OnDragEnter(MyGeometry, DragDropEvent);
	}
}

TSharedRef<SWidget> SInventoryPaletteItem::CreateTextSlotWidget(FCreateWidgetForActionData* const InCreateData, TAttribute<bool> bIsReadOnly)
{
	FName const ActionTypeId = InCreateData->Action->GetTypeId();

	FOnVerifyTextChanged OnVerifyTextChanged;
	FOnTextCommitted     OnTextCommitted;

	// default to our own rename methods
	OnVerifyTextChanged.BindSP(this, &SInventoryPaletteItem::OnNameTextVerifyChanged);
	OnTextCommitted.BindSP(this, &SInventoryPaletteItem::OnNameTextCommitted);

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
				.Text(this, &SInventoryPaletteItem::GetDisplayText)
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

FText SInventoryPaletteItem::GetDisplayText() const
{
	const UEdGraphSchema_OrionInventory* InventorySchema = GetDefault<UEdGraphSchema_OrionInventory>();
	if (MenuDescriptionCache.IsOutOfDate(InventorySchema))
	{
		TSharedPtr< FEdGraphSchemaAction > GraphAction = ActionPtr.Pin();
		FAssetSchemaAction_InventoryGraph* InventoryGraphAction = (FAssetSchemaAction_InventoryGraph*)GraphAction.Get();
		if (UOrionInventoryEdGraph* InventoryEdGraph = Cast<UOrionInventoryEdGraph>(InventoryGraphAction->EdGraph))
		{
			MenuDescriptionCache.SetCachedText(FText::FromString("Inventory Graph"), InventorySchema);
		}
	}

	return MenuDescriptionCache;
}

bool SInventoryPaletteItem::OnNameTextVerifyChanged(const FText& InNewText, FText& OutErrorMessage)
{
	FString FinalString = InNewText.ToString().TrimStartAndEnd();

	FName OriginalName;

	FAssetSchemaAction_InventoryGraph* InventoryGraphAction = (FAssetSchemaAction_InventoryGraph*)ActionPtr.Pin().Get();
	

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> InventoryGraphDataArray;
	AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UOrionInventoryGraph::StaticClass()), InventoryGraphDataArray);

	
	UStruct* ValidationScope = nullptr;

	const UEdGraphSchema* Schema = nullptr;

	return true;
}

#include "GameFramework/Actor.h"
#include "Logging/LogMacros.h"

void SInventoryPaletteItem::OnNameTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit)  
{  
  const FString NewNameString = NewText.ToString().TrimStartAndEnd();  
  const FName NewName = *NewNameString;  
  FName OriginalName;  

  FAssetSchemaAction_InventoryGraph* InventoryGraphAction = (FAssetSchemaAction_InventoryGraph*)ActionPtr.Pin().Get();  

  InventoryEditorPtr.Pin()->GetMyInventoryWidget()->Refresh();  

  InventoryEditorPtr.Pin()->GetOrionInventoryGraph()->Modify();  
}

TSharedPtr<SToolTip> SInventoryPaletteItem::ConstructToolTipWidget() const
{

	return TSharedPtr<SToolTip>();
}

FText SInventoryPaletteItem::GetToolTipText() const
{
	return FText::FromString(TEXT(""));
}

void SInventoryPalette::Construct(const FArguments& InArgs, TWeakPtr<FOrionInventoryEditor> InInventoryEditor)
{
}

void SInventoryPalette::OnSplitterResized() const
{
}

#undef LOCTEXT_NAMESPACE