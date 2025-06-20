// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "SMyInventory.h"
#include "Framework/Commands/GenericCommands.h"
#include "GraphEditorActions.h"
#include "SGraphActionMenu.h"
#include "GraphActionNode.h"
#include "SGraphPalette.h"
#include "OrionInventoryEditorUtils.h"
#include "OrionInventory_EditorCommands.h"
#include "Widgets/Input/SSearchBox.h"
#include "OrionInventoryEditorModule.h"
#include "OrionInventory_EditorCommands.h"
#include "SlateOptMacros.h"
#include "SInventoryPalette.h"


#define LOCTEXT_NAMESPACE "MyInventory"

void FMyInventoryCommands::RegisterCommands()
{
	UI_COMMAND(OpenGraph, "Open Graph", "Opens up this function, macro, or event graph's graph panel up.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenGraphInNewTab, "Open in New Tab", "Opens up this function, macro, or event graph's graph panel up in a new tab. Hold down Ctrl and double click for shortcut.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenExternalGraph, "Open External Graph", "Opens up this external graph's graph panel in its own asset editor", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(FocusNode, "Focus", "Focuses on the associated node", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(FocusNodeInNewTab, "Focus in New Tab", "Focuses on the associated node in a new tab", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(DeleteEntry, "Delete", "Deletes this function or variable from this Inventory Editor.", EUserInterfaceActionType::Button, FInputChord(EKeys::Platform_Delete));
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SMyInventory::Construct(const FArguments& InArgs, TWeakPtr<FOrionInventoryEditor> InInventoryEditor, const UOrionInventoryGraph* InInventorySystemGraph)
{
	bNeedsRefresh = false;
	bShowReplicatedVariablesOnly = false;

	InventoryEditorPtr = InInventoryEditor;
	EdGraph = nullptr;

	TSharedPtr<SWidget> ToolbarBuilderWidget = TSharedPtr<SWidget>();

	if (InInventoryEditor.IsValid())
	{
		OrionInventoryGraph = InventoryEditorPtr.Pin()->GetOrionInventoryGraph();

		CommandList = MakeShareable(new FUICommandList);

		CommandList->Append(InInventoryEditor.Pin()->GetToolkitCommands());

		CommandList->MapAction(FMyInventoryCommands::Get().OpenGraph,
			FExecuteAction::CreateSP(this, &SMyInventory::OnOpenGraph),
			FCanExecuteAction(), FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &SMyInventory::CanOpenGraph));

		CommandList->MapAction(FMyInventoryCommands::Get().OpenGraphInNewTab,
			FExecuteAction::CreateSP(this, &SMyInventory::OnOpenGraphInNewTab),
			FCanExecuteAction(), FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &SMyInventory::CanOpenGraph));

		CommandList->MapAction(FMyInventoryCommands::Get().OpenExternalGraph,
			FExecuteAction::CreateSP(this, &SMyInventory::OnOpenExternalGraph),
			FCanExecuteAction(), FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &SMyInventory::CanOpenExternalGraph));

		CommandList->MapAction(FMyInventoryCommands::Get().FocusNode,
			FExecuteAction::CreateSP(this, &SMyInventory::OnFocusNode),
			FCanExecuteAction(), FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &SMyInventory::CanFocusOnNode));

		CommandList->MapAction(FMyInventoryCommands::Get().FocusNodeInNewTab,
			FExecuteAction::CreateSP(this, &SMyInventory::OnFocusNodeInNewTab),
			FCanExecuteAction(), FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &SMyInventory::CanFocusOnNode));


		CommandList->MapAction(FMyInventoryCommands::Get().DeleteEntry,
			FExecuteAction::CreateSP(this, &SMyInventory::OnDeleteEntry),
			FCanExecuteAction::CreateSP(this, &SMyInventory::CanDeleteEntry));

		CommandList->MapAction(FGenericCommands::Get().Duplicate,
			FExecuteAction::CreateSP(this, &SMyInventory::OnDuplicateAction),
			FCanExecuteAction::CreateSP(this, &SMyInventory::CanDuplicateAction),
			FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &SMyInventory::IsDuplicateActionVisible));

		ToolbarBuilderWidget = SNullWidget::NullWidget;

		CommandList->MapAction(FGenericCommands::Get().Rename,
			FExecuteAction::CreateSP(this, &SMyInventory::OnRequestRenameOnActionNode),
			FCanExecuteAction::CreateSP(this, &SMyInventory::CanRequestRenameOnActionNode));

		CommandList->MapAction(FGenericCommands::Get().Copy,
			FExecuteAction::CreateSP(this, &SMyInventory::OnCopy),
			FCanExecuteAction::CreateSP(this, &SMyInventory::CanCopy));

		CommandList->MapAction(FGenericCommands::Get().Cut,
			FExecuteAction::CreateSP(this, &SMyInventory::OnCut),
			FCanExecuteAction::CreateSP(this, &SMyInventory::CanCut));

		CommandList->MapAction(FGenericCommands::Get().Paste,
			FExecuteAction::CreateSP(this, &SMyInventory::OnPasteGeneric),
			FCanExecuteAction(), FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &SMyInventory::CanPasteGeneric));

	}
	else
	{
		// we're in read only mode when there's no inventory editor:
		OrionInventoryGraph = const_cast<UOrionInventoryGraph*>(InInventorySystemGraph);
		check(OrionInventoryGraph);
		ToolbarBuilderWidget = SNew(SBox);
	}


	SAssignNew(FilterBox, SSearchBox)
		.OnTextChanged(this, &SMyInventory::OnFilterTextChanged);

	// create the main action list piece of this widget
	SAssignNew(GraphActionMenu, SGraphActionMenu, false)
		.OnGetFilterText(this, &SMyInventory::GetFilterText)
		.OnCreateWidgetForAction(this, &SMyInventory::OnCreateWidgetForAction)
		.OnCollectAllActions(this, &SMyInventory::CollectAllActions)
		.OnCollectStaticSections(this, &SMyInventory::CollectStaticSections)
		.OnActionDragged(this, &SMyInventory::OnActionDragged)
		.OnCategoryDragged(this, &SMyInventory::OnCategoryDragged)
		.OnActionSelected(this, &SMyInventory::OnGlobalActionSelected)
		.OnActionDoubleClicked(this, &SMyInventory::OnActionDoubleClicked)
		.OnContextMenuOpening(this, &SMyInventory::OnContextMenuOpening)
		.OnCategoryTextCommitted(this, &SMyInventory::OnCategoryNameCommitted)
		.OnCanRenameSelectedAction(this, &SMyInventory::CanRequestRenameOnActionNode)
		.OnGetSectionTitle(this, &SMyInventory::OnGetSectionTitle)
		.OnGetSectionWidget(this, &SMyInventory::OnGetSectionWidget)
		.OnActionMatchesName(this, &SMyInventory::HandleActionMatchesName)
		.AlphaSortItems(false)
		.UseSectionStyling(true);

	ChildSlot
	[
		SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
					.Padding(4.0f)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
					.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("MyInventoryPanel")))
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							ToolbarBuilderWidget.ToSharedRef()
						]
					]
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				GraphActionMenu.ToSharedRef()
			]
	];

	//ResetLastPinType();

	if (!InventoryEditorPtr.IsValid())
	{
		Refresh();
	}

	TMap<int32, bool> ExpandedSections;
	ExpandedSections.Add(InventorySectionID::InventoryDatabase, true);

	GraphActionMenu->SetSectionExpansion(ExpandedSections);

	FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(this, &SMyInventory::OnObjectPropertyChanged);

}

SMyInventory::~SMyInventory()
{
	FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
}
void SMyInventory::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (bNeedsRefresh)
	{
		Refresh();
	}
}

FReply SMyInventory::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (CommandList.IsValid() && CommandList->ProcessCommandBindings(InKeyEvent))
	{
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

template<class SchemaActionType> SchemaActionType* SelectionAsType(const TSharedPtr< SGraphActionMenu >& GraphActionMenu)
{
	TArray<TSharedPtr<FEdGraphSchemaAction> > SelectedActions;
	GraphActionMenu->GetSelectedActions(SelectedActions);

	SchemaActionType* Selection = NULL;

	TSharedPtr<FEdGraphSchemaAction> SelectedAction(SelectedActions.Num() > 0 ? SelectedActions[0] : NULL);
	if (SelectedAction.IsValid() &&
		SelectedAction->GetTypeId() == SchemaActionType::StaticGetTypeId())
	{
		// TODO Why not? StaticCastSharedPtr<>()

		Selection = (SchemaActionType*)SelectedActions[0].Get();
	}

	return Selection;
}

FAssetSchemaAction_InventoryGraph* SMyInventory::SelectionAsGraph() const
{
	return SelectionAsType<FAssetSchemaAction_InventoryGraph>(GraphActionMenu);
}
void SMyInventory::OnRequestRenameOnActionNode()
{
	// Attempt to rename in both menus, only one of them will have anything selected
	GraphActionMenu->OnRequestRenameOnActionNode();
	if (InventoryEditorPtr.IsValid())
	{

	}
}
void SMyInventory::OnPasteGeneric()
{
}
bool SMyInventory::CanPasteGeneric()
{
	return false;
}
void SMyInventory::Refresh()
{
	bNeedsRefresh = false;

	// Conform to our interfaces here to ensure we catch any newly added functions
	//FBlueprintEditorUtils::ConformImplementedInterfaces(GetOrionInventoryGraph());

	GraphActionMenu->RefreshAllActions(/*bPreserveExpansion=*/ true);
}

bool SMyInventory::SelectionIsCategory() const
{
	return !SelectionHasContextMenu();
}

void SMyInventory::SelectItemByName(const FName& ItemName, ESelectInfo::Type SelectInfo, int32 SectionId, bool bIsCategory)
{
	// Check if the graph action menu is being told to clear
	if (ItemName == NAME_None)
	{
		ClearGraphActionMenuSelection();
	}
	else
	{
		// Attempt to select the item in the main graph action menu
		const bool bSucceededAtSelecting = GraphActionMenu->SelectItemByName(ItemName, SelectInfo, SectionId, bIsCategory);
		if (!bSucceededAtSelecting)
		{
			// We failed to select the item, maybe because it was filtered out?
			// Reset the item filter and try again (we don't do this first because someone went to the effort of typing
			// a filter and probably wants to keep it unless it is getting in the way, as it just has)
			//OnResetItemFilter();
			GraphActionMenu->SelectItemByName(ItemName, SelectInfo, SectionId, bIsCategory);
		}
	}
}

void SMyInventory::ClearGraphActionMenuSelection()
{
	GraphActionMenu->SelectItemByName(NAME_None);
}


TSharedRef<SWidget> SMyInventory::OnCreateWidgetForAction(FCreateWidgetForActionData* const InCreateData)
{
	return InventoryEditorPtr.IsValid() ? SNew(SInventoryPaletteItem, InCreateData, InventoryEditorPtr.Pin()) : SNew(SInventoryPaletteItem, InCreateData, GetOrionInventoryGraph());
}

void SMyInventory::CollectAllActions(FGraphActionListBuilderBase& OutAllActions)
{
	UOrionInventoryGraph* InventorySystemObj = GetOrionInventoryGraph();
	check(InventorySystemObj);

	TSharedPtr<FOrionInventoryEditor> InventorySystemEditor = InventoryEditorPtr.Pin();

	EFieldIteratorFlags::SuperClassFlags FieldIteratorSuperFlag = EFieldIteratorFlags::IncludeSuper;

	if (InventorySystemEditor.IsValid())
	{
		// Grab ubergraph pages
		for (int32 i = 0; i < InventorySystemObj->InventoryGraphPages.Num(); i++)
		{
			UEdGraph* Graph = InventorySystemObj->InventoryGraphPages[i];
			check(Graph);

			FGraphDisplayInfo DisplayInfo;
			Graph->GetSchema()->GetGraphDisplayInformation(*Graph, DisplayInfo);

			TSharedPtr<FAssetSchemaAction_InventoryGraph> NeUbergraphAction = MakeShareable(new FAssetSchemaAction_InventoryGraph(FText::GetEmpty(), DisplayInfo.PlainName, DisplayInfo.Tooltip, 2, InventorySectionID::InventoryDatabase));
			NeUbergraphAction->FuncName = Graph->GetFName();
			NeUbergraphAction->EdGraph = Graph;
			OutAllActions.AddAction(NeUbergraphAction);
		}
	}
}
void SMyInventory::CollectStaticSections(TArray<int32>& StaticSectionIDs)
{
	StaticSectionIDs.Add(InventorySectionID::InventoryDatabase);
}
FReply SMyInventory::OnActionDragged(const TArray<TSharedPtr<FEdGraphSchemaAction>>& InActions, const FPointerEvent& MouseEvent)
{
	return FReply::Handled();
}
FReply SMyInventory::OnCategoryDragged(const FText& InCategory, const FPointerEvent& MouseEvent)
{
	return FReply::Handled();
}
void SMyInventory::OnActionSelected(const TArray<TSharedPtr<FEdGraphSchemaAction>>& InActions)
{
	TSharedPtr<FEdGraphSchemaAction> InAction(InActions.Num() > 0 ? InActions[0] : NULL);
	if (InAction.IsValid())
	{
		FAssetSchemaAction_InventoryGraph* GraphAction = (FAssetSchemaAction_InventoryGraph*)InAction.Get();
		InventoryEditorPtr.Pin()->FocusedEdGraph = GraphAction->EdGraph;

		if (GraphAction->EdGraph)
		{
			TSharedPtr<FOrionInventoryEditor> InventoryEditor = InventoryEditorPtr.Pin();

			if (UOrionInventoryEdGraph* InventoryGraph = Cast<UOrionInventoryEdGraph>(InventoryEditor->FocusedEdGraph))
			{
				InventoryEditor->PropertyWidget->SetObject(InventoryGraph->OwningInventory);
			}
		}
	}
}

void SMyInventory::OnGlobalActionSelected(const TArray<TSharedPtr<FEdGraphSchemaAction>>& InActions, ESelectInfo::Type InSelectionType)
{
	if (InSelectionType == ESelectInfo::OnMouseClick || InSelectionType == ESelectInfo::OnKeyPress || InSelectionType == ESelectInfo::OnNavigation || InActions.Num() == 0)
	{
		OnActionSelected(InActions);

	}
}

void SMyInventory::OnActionDoubleClicked(const TArray<TSharedPtr<FEdGraphSchemaAction>>& InActions)
{
	if (!InventoryEditorPtr.IsValid())
	{
		return;
	}

	TSharedPtr<FEdGraphSchemaAction> InAction(InActions.Num() > 0 ? InActions[0] : NULL);
	ExecuteAction(InAction);

}

void SMyInventory::ExecuteAction(TSharedPtr<FEdGraphSchemaAction> InAction)
{
	// Force it to open in a new document if shift is pressed
	const bool bIsShiftPressed = FSlateApplication::Get().GetModifierKeys().IsShiftDown();
	FDocumentTracker::EOpenDocumentCause OpenMode = bIsShiftPressed ? FDocumentTracker::ForceOpenNewDocument : FDocumentTracker::OpenNewDocument;

	UOrionInventoryGraph* InventorySystemObj = InventoryEditorPtr.Pin()->GetOrionInventoryGraph();
	if (InAction.IsValid())
	{
		if (InAction->GetTypeId() == FAssetSchemaAction_InventoryGraph::StaticGetTypeId())
		{
			FAssetSchemaAction_InventoryGraph* GraphAction = (FAssetSchemaAction_InventoryGraph*)InAction.Get();
			InventoryEditorPtr.Pin()->FocusedEdGraph = GraphAction->EdGraph;

			if (GraphAction->EdGraph)
			{
				InventoryEditorPtr.Pin()->JumpToHyperlink(GraphAction->EdGraph);

				TSharedPtr<FOrionInventoryEditor> InventoryEditor = InventoryEditorPtr.Pin();

				if (UOrionInventoryEdGraph* InventoryGraph = Cast<UOrionInventoryEdGraph>(InventoryEditor->CurrentGraphWidget->GetCurrentGraph()))
				{
					InventoryEditor->PropertyWidget->SetObject(InventoryGraph->OwningInventory);
				}
			}
		}
	}
}

TSharedPtr<SWidget> SMyInventory::OnContextMenuOpening()
{
	if (!InventoryEditorPtr.IsValid())
	{
		return TSharedPtr<SWidget>();
	}

	const bool bShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, CommandList);

	// Check if the selected action is valid for a context menu
	if (SelectionHasContextMenu())
	{
		MenuBuilder.BeginSection("BasicOperations");
		{
			MenuBuilder.AddMenuEntry(FMyInventoryCommands::Get().OpenGraph);
			MenuBuilder.AddMenuEntry(FMyInventoryCommands::Get().OpenGraphInNewTab);
			MenuBuilder.AddMenuEntry(FMyInventoryCommands::Get().OpenExternalGraph);
			MenuBuilder.AddMenuEntry(FMyInventoryCommands::Get().FocusNode);
			MenuBuilder.AddMenuEntry(FMyInventoryCommands::Get().FocusNodeInNewTab);
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Rename, NAME_None, LOCTEXT("Rename", "Rename"), LOCTEXT("Rename_Tooltip", "Rename Inventory"));
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Cut);
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Copy);
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Duplicate);
			MenuBuilder.AddMenuEntry(FMyInventoryCommands::Get().DeleteEntry);
		}
		MenuBuilder.EndSection();

		FAssetSchemaAction_InventoryGraph* Graph = SelectionAsGraph();

		//if (Var && BlueprintEditorPtr.IsValid() && FBlueprintEditorUtils::DoesSupportEventGraphs(GetBlueprintObj()))
		//{
		//	FObjectProperty* ComponentProperty = CastField<FObjectProperty>(Var->GetProperty());

		//	if (ComponentProperty && ComponentProperty->PropertyClass &&
		//		ComponentProperty->PropertyClass->IsChildOf(UActorComponent::StaticClass()))
		//	{
		//		if (FBlueprintEditorUtils::CanClassGenerateEvents(ComponentProperty->PropertyClass))
		//		{
		//			TSharedPtr<FBlueprintEditor> BlueprintEditor(BlueprintEditorPtr.Pin());

		//			// If the selected item is valid, and is a component of some sort, build a context menu
		//			// of events appropriate to the component.
		//			MenuBuilder.AddSubMenu(LOCTEXT("AddEventSubMenu", "Add Event"),
		//				LOCTEXT("AddEventSubMenu_ToolTip", "Add Event"),
		//				FNewMenuDelegate::CreateStatic(&SSubobjectBlueprintEditor::BuildMenuEventsSection,
		//					BlueprintEditor->GetBlueprintObj(), ComponentProperty->PropertyClass,
		//					FCanExecuteAction::CreateRaw(this, &SMyBlueprint::IsEditingMode),
		//					FGetSelectedObjectsDelegate::CreateSP(this, &SMyBlueprint::GetSelectedItemsForContextMenu)));
		//		}
		//	}
		//}
		
	}
	else
	{
		BuildAddNewMenu(MenuBuilder);
	}

	return MenuBuilder.MakeWidget();
}
void SMyInventory::BuildAddNewMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("AddNewItem", LOCTEXT("AddOperations", "Add New"));

	if (UOrionInventoryGraph* CurrentBlueprint = GetOrionInventoryGraph())
	{
		MenuBuilder.AddMenuEntry(FOrionInventory_EditorCommands::Get().AddNewInventoryGraph);
		
	}
	MenuBuilder.EndSection();
}
TSharedRef<SWidget> SMyInventory::CreateAddToSectionButton(int32 InSectionID, TWeakPtr<SWidget> WeakRowWidget, FText AddNewText, FName MetaDataTag)
{
	return
		SNew(SButton)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.OnClicked(this, &SMyInventory::OnAddButtonClickedOnSection, InSectionID)
		.IsEnabled(this, &SMyInventory::CanAddNewElementToSection, InSectionID)
		.ContentPadding(FMargin(1, 0))
		.AddMetaData<FTagMetaData>(FTagMetaData(MetaDataTag))
		.ToolTipText(AddNewText)
		[
			SNew(SImage)
				.Image(FAppStyle::Get().GetBrush("Icons.PlusCircle"))
				.ColorAndOpacity(FSlateColor::UseForeground())
		];
}
void SMyInventory::OnCategoryNameCommitted(const FText& InNewText, ETextCommit::Type InTextCommit, TWeakPtr<struct FGraphActionNode> InAction)
{
	// Remove excess whitespace and prevent categories with just spaces
	FText CategoryName = FText::TrimPrecedingAndTrailing(InNewText);

	TArray<TSharedPtr<FEdGraphSchemaAction>> Actions;
	GraphActionMenu->GetCategorySubActions(InAction, Actions);

	if (Actions.Num())
	{
		const FScopedTransaction Transaction(LOCTEXT("RenameCategory", "Rename Category"));

		GetOrionInventoryGraph()->Modify();

		Refresh();
		SelectItemByName(FName(*CategoryName.ToString()), ESelectInfo::OnMouseClick, InAction.Pin()->SectionID, true);
	}
}
bool SMyInventory::CanRequestRenameOnActionNode(TWeakPtr<struct FGraphActionNode> InSelectedNode) const
{
	return false;
}

FText SMyInventory::OnGetSectionTitle(int32 InSectionID)
{
	FText SeperatorTitle;
	/* Setup an appropriate name for the section for this node */
	switch (InSectionID)
	{
	case InventorySectionID::InventoryDatabase:
		SeperatorTitle = NSLOCTEXT("GraphActionNode", "Inventory Graph", "Inventory Graph");
		break; 
	default:
	case InventorySectionID::NONE:
		SeperatorTitle = FText::GetEmpty();
		break;
	}
	
	return SeperatorTitle;
}
TSharedRef<SWidget> SMyInventory::OnGetSectionWidget(TSharedRef<SWidget> RowWidget, int32 InSectionID)
{
	TWeakPtr<SWidget> WeakRowWidget = RowWidget;

	FText AddNewText;
	FName MetaDataTag;

	switch (InSectionID)
	{
	case InventorySectionID::InventoryDatabase:
		AddNewText = LOCTEXT("AddNewInventoryGraph", "New Inventory Graph");
		MetaDataTag = TEXT("AddNewGraph");
		break;
	default:
		return SNullWidget::NullWidget;
	}

	return CreateAddToSectionButton(InSectionID, WeakRowWidget, AddNewText, MetaDataTag);
}
FReply SMyInventory::OnAddButtonClickedOnSection(int32 InSectionID)
{
	TSharedPtr<FOrionInventoryEditor> InventoryEditor = InventoryEditorPtr.Pin();

	switch (InSectionID)
	{
	case InventorySectionID::InventoryDatabase:
		CommandList->ExecuteAction(FOrionInventory_EditorCommands::Get().AddNewInventoryGraph.ToSharedRef());
		break;
	}

	return FReply::Handled();
}
bool SMyInventory::CanAddNewElementToSection(int32 InSectionID) const
{
	return false;
	// true if we want to add new graph section
	//return true;
}
bool SMyInventory::HandleActionMatchesName(FEdGraphSchemaAction* InAction, const FName& InName) const
{
	return false;
}
void SMyInventory::OnOpenGraph()
{
	OpenGraph(FDocumentTracker::OpenNewDocument);
}
void SMyInventory::OnOpenGraphInNewTab()
{
	OpenGraph(FDocumentTracker::ForceOpenNewDocument);
}
void SMyInventory::OnOpenExternalGraph()
{
}
bool SMyInventory::CanOpenGraph() const
{
	return true;
}
bool SMyInventory::CanOpenExternalGraph() const
{
	return false;
}
bool SMyInventory::CanFocusOnNode() const
{
	return false;
}
void SMyInventory::OnFocusNode()
{
}
void SMyInventory::OnFocusNodeInNewTab()
{
}
void SMyInventory::OnDeleteEntry()
{
	if (FAssetSchemaAction_InventoryGraph* GraphAction = SelectionAsGraph())
	{
		OnDeleteGraph(GraphAction->EdGraph);
	}

	Refresh();
	//BlueprintEditorPtr.Pin()->GetInspector()->ShowDetailsForObjects(TArray<UObject*>());

}

bool SMyInventory::CanDeleteEntry() const
{
	/*if (FAssetSchemaAction_InventoryGraph* GraphAction = SelectionAsGraph())
	{
		return (GraphAction->EdGraph ? GraphAction->EdGraph->bAllowDeletion : false);
	}*/
	return false;
}

bool SMyInventory::CanRequestRenameOnActionNode() const
{
	TArray<TSharedPtr<FEdGraphSchemaAction> > SelectedActions;
	GraphActionMenu->GetSelectedActions(SelectedActions);

	// If there is anything selected in the GraphActionMenu, check the item for if it can be renamed.
	if (SelectedActions.Num() || SelectionIsCategory())
	{
		return GraphActionMenu->CanRequestRenameOnActionNode();
	}
	return false;
}

bool SMyInventory::IsDuplicateActionVisible() const
{
	return false;
}
bool SMyInventory::CanDuplicateAction() const
{
	return false;
}
void SMyInventory::OnDuplicateAction()
{
}



void SMyInventory::OnCopy()
{
	
}
bool SMyInventory::CanCopy() const
{
	return false;
}
void SMyInventory::OnCut()
{
}
bool SMyInventory::CanCut() const
{
	return false;
}

void SMyInventory::OnDeleteGraph(UEdGraph* InGraph)
{
	if (InGraph && InGraph->bAllowDeletion)
	{
		if (const UEdGraphSchema* Schema = InGraph->GetSchema())
		{
			if (Schema->TryDeleteGraph(InGraph))
			{
				return;
			}
		}

		const FScopedTransaction Transaction(LOCTEXT("RemoveGraph", "Remove Graph"));
		GetOrionInventoryGraph()->Modify();

		InGraph->Modify();

		
		// Remove any  nodes bound to this graph
		TArray<UOrionInventoryEdNode*> AllNodes;
		FOrionInventoryEditorUtils::GetAllNodesOfClass<UOrionInventoryEdNode>(GetOrionInventoryGraph(), AllNodes);

		const bool bDontRecompile = true;
		for (UOrionInventoryEdNode* CompNode : AllNodes)
		{
			if (CompNode->GetOrionInventoryEdGraph() == InGraph)
			{
				FOrionInventoryEditorUtils::RemoveNode(GetOrionInventoryGraph(), CompNode, bDontRecompile);
			}
		}
		

		FOrionInventoryEditorUtils::RemoveGraph(GetOrionInventoryGraph(), InGraph);
		InventoryEditorPtr.Pin()->CloseDocumentTab(InGraph);


		InGraph = NULL;
	}
}

void SMyInventory::OpenGraph(FDocumentTracker::EOpenDocumentCause InCause, bool bOpenExternalGraphInNewEditor)
{
	UEdGraph* GraphToOpen = nullptr;

	if (FAssetSchemaAction_InventoryGraph* GraphAction = SelectionAsGraph())
	{
		GraphToOpen = GraphAction->EdGraph;
		// If we have no graph then this is an interface event, so focus on the event graph
		/*if (!GraphToOpen)
		{
			GraphToOpen = FOrionInventoryEditorUtils::FindInventoryGraph(GetOrionInventoryGraph());
		}*/
	}

	if (GraphToOpen)
	{
		if (bOpenExternalGraphInNewEditor )
		{
			InventoryEditorPtr.Pin()->JumpToHyperlink(GraphToOpen, false);
		}
		else
		{
			InventoryEditorPtr.Pin()->OpenDocument(GraphToOpen, InCause);
		}
	}
}

bool SMyInventory::SelectionHasContextMenu() const
{
	TArray<TSharedPtr<FEdGraphSchemaAction> > SelectedActions;
	GraphActionMenu->GetSelectedActions(SelectedActions);
	return SelectedActions.Num() > 0;
}

void SMyInventory::OnObjectPropertyChanged(UObject* InObject, FPropertyChangedEvent& InPropertyChangedEvent)
{
	if (InObject == OrionInventoryGraph && (InPropertyChangedEvent.ChangeType != EPropertyChangeType::ValueSet && InPropertyChangedEvent.ChangeType != EPropertyChangeType::ArrayClear))
	{
		bNeedsRefresh = true;
	}
}

void SMyInventory::OnFilterTextChanged(const FText& InFilterText)
{
}
FText SMyInventory::GetFilterText() const
{
	return FText();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION



#undef LOCTEXT_NAMESPACE