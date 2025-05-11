// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "SMyDialog.h"
#include "Framework/Commands/GenericCommands.h"
#include "GraphEditorActions.h"
#include "SGraphActionMenu.h"
#include "GraphActionNode.h"
#include "SGraphPalette.h"
#include "DialogBuilderEditorUtils.h"
#include "DialogBuilder_EditorCommands.h"
#include "Widgets/Input/SSearchBox.h"
#include "DialogBuilderEditor.h"
#include "DialogBuilder_EditorCommands.h"
#include "SlateOptMacros.h"
#include "SDialogPalette.h"


#define LOCTEXT_NAMESPACE "MyQuest"

void FMyDialogCommands::RegisterCommands()
{
	UI_COMMAND(OpenGraph, "Open Graph", "Opens up this function, macro, or event graph's graph panel up.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenGraphInNewTab, "Open in New Tab", "Opens up this function, macro, or event graph's graph panel up in a new tab. Hold down Ctrl and double click for shortcut.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenExternalGraph, "Open External Graph", "Opens up this external graph's graph panel in its own asset editor", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(FocusNode, "Focus", "Focuses on the associated node", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(FocusNodeInNewTab, "Focus in New Tab", "Focuses on the associated node in a new tab", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(DeleteEntry, "Delete", "Deletes this function or variable from this Dialog Editor.", EUserInterfaceActionType::Button, FInputChord(EKeys::Platform_Delete));
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SMyDialog::Construct(const FArguments& InArgs, TWeakPtr<FDialogBuilderEditor> InQuestEditor, const UQuestBuilderGraph* InQuestSystemGraph)
{
	bNeedsRefresh = false;
	bShowReplicatedVariablesOnly = false;

	QuestEditorPtr = InQuestEditor;
	EdGraph = nullptr;

	TSharedPtr<SWidget> ToolbarBuilderWidget = TSharedPtr<SWidget>();

	if (InQuestEditor.IsValid())
	{
		QuestBuilderGraph = QuestEditorPtr.Pin()->GetQuestBuilderGraph();

		CommandList = MakeShareable(new FUICommandList);

		CommandList->Append(InQuestEditor.Pin()->GetToolkitCommands());

		CommandList->MapAction(FMyDialogCommands::Get().OpenGraph,
			FExecuteAction::CreateSP(this, &SMyDialog::OnOpenGraph),
			FCanExecuteAction(), FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &SMyDialog::CanOpenGraph));

		CommandList->MapAction(FMyDialogCommands::Get().OpenGraphInNewTab,
			FExecuteAction::CreateSP(this, &SMyDialog::OnOpenGraphInNewTab),
			FCanExecuteAction(), FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &SMyDialog::CanOpenGraph));

		CommandList->MapAction(FMyDialogCommands::Get().OpenExternalGraph,
			FExecuteAction::CreateSP(this, &SMyDialog::OnOpenExternalGraph),
			FCanExecuteAction(), FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &SMyDialog::CanOpenExternalGraph));

		CommandList->MapAction(FMyDialogCommands::Get().FocusNode,
			FExecuteAction::CreateSP(this, &SMyDialog::OnFocusNode),
			FCanExecuteAction(), FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &SMyDialog::CanFocusOnNode));

		CommandList->MapAction(FMyDialogCommands::Get().FocusNodeInNewTab,
			FExecuteAction::CreateSP(this, &SMyDialog::OnFocusNodeInNewTab),
			FCanExecuteAction(), FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &SMyDialog::CanFocusOnNode));


		CommandList->MapAction(FMyDialogCommands::Get().DeleteEntry,
			FExecuteAction::CreateSP(this, &SMyDialog::OnDeleteEntry),
			FCanExecuteAction::CreateSP(this, &SMyDialog::CanDeleteEntry));

		CommandList->MapAction(FGenericCommands::Get().Duplicate,
			FExecuteAction::CreateSP(this, &SMyDialog::OnDuplicateAction),
			FCanExecuteAction::CreateSP(this, &SMyDialog::CanDuplicateAction),
			FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &SMyDialog::IsDuplicateActionVisible));

		ToolbarBuilderWidget = SNullWidget::NullWidget;

		CommandList->MapAction(FGenericCommands::Get().Rename,
			FExecuteAction::CreateSP(this, &SMyDialog::OnRequestRenameOnActionNode),
			FCanExecuteAction::CreateSP(this, &SMyDialog::CanRequestRenameOnActionNode));

		CommandList->MapAction(FGenericCommands::Get().Copy,
			FExecuteAction::CreateSP(this, &SMyDialog::OnCopy),
			FCanExecuteAction::CreateSP(this, &SMyDialog::CanCopy));

		CommandList->MapAction(FGenericCommands::Get().Cut,
			FExecuteAction::CreateSP(this, &SMyDialog::OnCut),
			FCanExecuteAction::CreateSP(this, &SMyDialog::CanCut));

		CommandList->MapAction(FGenericCommands::Get().Paste,
			FExecuteAction::CreateSP(this, &SMyDialog::OnPasteGeneric),
			FCanExecuteAction(), FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &SMyDialog::CanPasteGeneric));

	}
	else
	{
		// we're in read only mode when there's no quest editor:
		QuestBuilderGraph = const_cast<UQuestBuilderGraph*>(InQuestSystemGraph);
		check(QuestBuilderGraph);
		ToolbarBuilderWidget = SNew(SBox);
	}


	SAssignNew(FilterBox, SSearchBox)
		.OnTextChanged(this, &SMyDialog::OnFilterTextChanged);

	// create the main action list piece of this widget
	SAssignNew(GraphActionMenu, SGraphActionMenu, false)
		.OnGetFilterText(this, &SMyDialog::GetFilterText)
		.OnCreateWidgetForAction(this, &SMyDialog::OnCreateWidgetForAction)
		.OnCollectAllActions(this, &SMyDialog::CollectAllActions)
		.OnCollectStaticSections(this, &SMyDialog::CollectStaticSections)
		.OnActionDragged(this, &SMyDialog::OnActionDragged)
		.OnCategoryDragged(this, &SMyDialog::OnCategoryDragged)
		.OnActionSelected(this, &SMyDialog::OnGlobalActionSelected)
		.OnActionDoubleClicked(this, &SMyDialog::OnActionDoubleClicked)
		.OnContextMenuOpening(this, &SMyDialog::OnContextMenuOpening)
		.OnCategoryTextCommitted(this, &SMyDialog::OnCategoryNameCommitted)
		.OnCanRenameSelectedAction(this, &SMyDialog::CanRequestRenameOnActionNode)
		.OnGetSectionTitle(this, &SMyDialog::OnGetSectionTitle)
		.OnGetSectionWidget(this, &SMyDialog::OnGetSectionWidget)
		.OnActionMatchesName(this, &SMyDialog::HandleActionMatchesName)
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
					.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("MyQuestPanel")))
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							ToolbarBuilderWidget.ToSharedRef()
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.VAlign(VAlign_Center)
							[
								FilterBox.ToSharedRef()
							]
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

	if (!QuestEditorPtr.IsValid())
	{
		Refresh();
	}

	TMap<int32, bool> ExpandedSections;
	ExpandedSections.Add(DialogSectionID::DIALOGGRAPH, true);

	GraphActionMenu->SetSectionExpansion(ExpandedSections);

	FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(this, &SMyDialog::OnObjectPropertyChanged);

}

SMyDialog::~SMyDialog()
{
	FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
}
void SMyDialog::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (bNeedsRefresh)
	{
		Refresh();
	}
}

FReply SMyDialog::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
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

FAssetSchemaAction_DialogBuilderGraph* SMyDialog::SelectionAsGraph() const
{
	return SelectionAsType<FAssetSchemaAction_DialogBuilderGraph>(GraphActionMenu);
}
void SMyDialog::OnRequestRenameOnActionNode()
{
	// Attempt to rename in both menus, only one of them will have anything selected
	GraphActionMenu->OnRequestRenameOnActionNode();
	if (QuestEditorPtr.IsValid())
	{

	}
}
void SMyDialog::OnPasteGeneric()
{
}
bool SMyDialog::CanPasteGeneric()
{
	return false;
}
void SMyDialog::Refresh()
{
	bNeedsRefresh = false;

	// Conform to our interfaces here to ensure we catch any newly added functions
	//FBlueprintEditorUtils::ConformImplementedInterfaces(GetQuestBuilderGraph());

	GraphActionMenu->RefreshAllActions(/*bPreserveExpansion=*/ true);
}

bool SMyDialog::SelectionIsCategory() const
{
	return !SelectionHasContextMenu();
}

void SMyDialog::SelectItemByName(const FName& ItemName, ESelectInfo::Type SelectInfo, int32 SectionId, bool bIsCategory)
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

void SMyDialog::ClearGraphActionMenuSelection()
{
	GraphActionMenu->SelectItemByName(NAME_None);
}


TSharedRef<SWidget> SMyDialog::OnCreateWidgetForAction(FCreateWidgetForActionData* const InCreateData)
{
	return QuestEditorPtr.IsValid() ? SNew(SDialogPaletteItem, InCreateData, QuestEditorPtr.Pin()) : SNew(SDialogPaletteItem, InCreateData, GetQuestBuilderGraph());
}

void SMyDialog::CollectAllActions(FGraphActionListBuilderBase& OutAllActions)
{
	UQuestBuilderGraph* QuestSystemObj = GetQuestBuilderGraph();
	check(QuestSystemObj);

	TSharedPtr<FDialogBuilderEditor> QuestSystemEditor = QuestEditorPtr.Pin();

	EFieldIteratorFlags::SuperClassFlags FieldIteratorSuperFlag = EFieldIteratorFlags::IncludeSuper;

	if (QuestSystemEditor.IsValid())
	{
		// Grab ubergraph pages
		for (int32 i = 0; i < QuestSystemObj->QuestGraphPages.Num(); i++)
		{
			UEdGraph* Graph = QuestSystemObj->QuestGraphPages[i];
			check(Graph);

			FGraphDisplayInfo DisplayInfo;
			Graph->GetSchema()->GetGraphDisplayInformation(*Graph, DisplayInfo);

			TSharedPtr<FAssetSchemaAction_DialogBuilderGraph> NeUbergraphAction = MakeShareable(new FAssetSchemaAction_DialogBuilderGraph(FText::GetEmpty(), DisplayInfo.PlainName, DisplayInfo.Tooltip, 2, DialogSectionID::DIALOGGRAPH));
			NeUbergraphAction->FuncName = Graph->GetFName();
			NeUbergraphAction->EdGraph = Graph;
			OutAllActions.AddAction(NeUbergraphAction);
		}
	}
}
void SMyDialog::CollectStaticSections(TArray<int32>& StaticSectionIDs)
{
	StaticSectionIDs.Add(DialogSectionID::DIALOGGRAPH);
}
FReply SMyDialog::OnActionDragged(const TArray<TSharedPtr<FEdGraphSchemaAction>>& InActions, const FPointerEvent& MouseEvent)
{
	return FReply::Handled();
}
FReply SMyDialog::OnCategoryDragged(const FText& InCategory, const FPointerEvent& MouseEvent)
{
	return FReply::Handled();
}
void SMyDialog::OnActionSelected(const TArray<TSharedPtr<FEdGraphSchemaAction>>& InActions)
{
	TSharedPtr<FEdGraphSchemaAction> InAction(InActions.Num() > 0 ? InActions[0] : NULL);
	if (InAction.IsValid())
	{
		FAssetSchemaAction_DialogBuilderGraph* GraphAction = (FAssetSchemaAction_DialogBuilderGraph*)InAction.Get();
		QuestEditorPtr.Pin()->FocusedEdGraph = GraphAction->EdGraph;

		if (GraphAction->EdGraph)
		{
			TSharedPtr<FDialogBuilderEditor> QuestEditor = QuestEditorPtr.Pin();

			if (UDialogBuilderEdGraph* QuestGraph = Cast<UDialogBuilderEdGraph>(QuestEditor->FocusedEdGraph))
			{
				QuestEditor->PropertyWidget->SetObject(QuestGraph->Dialog);
			}
		}
	}
}

void SMyDialog::OnGlobalActionSelected(const TArray<TSharedPtr<FEdGraphSchemaAction>>& InActions, ESelectInfo::Type InSelectionType)
{
	if (InSelectionType == ESelectInfo::OnMouseClick || InSelectionType == ESelectInfo::OnKeyPress || InSelectionType == ESelectInfo::OnNavigation || InActions.Num() == 0)
	{
		OnActionSelected(InActions);

	}
}

void SMyDialog::OnActionDoubleClicked(const TArray<TSharedPtr<FEdGraphSchemaAction>>& InActions)
{
	if (!QuestEditorPtr.IsValid())
	{
		return;
	}

	TSharedPtr<FEdGraphSchemaAction> InAction(InActions.Num() > 0 ? InActions[0] : NULL);
	ExecuteAction(InAction);

}

void SMyDialog::ExecuteAction(TSharedPtr<FEdGraphSchemaAction> InAction)
{
	// Force it to open in a new document if shift is pressed
	const bool bIsShiftPressed = FSlateApplication::Get().GetModifierKeys().IsShiftDown();
	FDocumentTracker::EOpenDocumentCause OpenMode = bIsShiftPressed ? FDocumentTracker::ForceOpenNewDocument : FDocumentTracker::OpenNewDocument;

	UQuestBuilderGraph* QuestSystemObj = QuestEditorPtr.Pin()->GetQuestBuilderGraph();
	if (InAction.IsValid())
	{
		if (InAction->GetTypeId() == FAssetSchemaAction_DialogBuilderGraph::StaticGetTypeId())
		{
			FAssetSchemaAction_DialogBuilderGraph* GraphAction = (FAssetSchemaAction_DialogBuilderGraph*)InAction.Get();
			QuestEditorPtr.Pin()->FocusedEdGraph = GraphAction->EdGraph;

			if (GraphAction->EdGraph)
			{
				QuestEditorPtr.Pin()->JumpToHyperlink(GraphAction->EdGraph);

				TSharedPtr<FDialogBuilderEditor> QuestEditor = QuestEditorPtr.Pin();

				if (UDialogBuilderEdGraph* QuestGraph = Cast<UDialogBuilderEdGraph>(QuestEditor->ViewportWidget->GetCurrentGraph()))
				{
					QuestEditor->PropertyWidget->SetObject(QuestGraph->Dialog);
				}
			}
		}
	}
}

TSharedPtr<SWidget> SMyDialog::OnContextMenuOpening()
{
	if (!QuestEditorPtr.IsValid())
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
			MenuBuilder.AddMenuEntry(FMyDialogCommands::Get().OpenGraph);
			MenuBuilder.AddMenuEntry(FMyDialogCommands::Get().OpenGraphInNewTab);
			MenuBuilder.AddMenuEntry(FMyDialogCommands::Get().OpenExternalGraph);
			MenuBuilder.AddMenuEntry(FMyDialogCommands::Get().FocusNode);
			MenuBuilder.AddMenuEntry(FMyDialogCommands::Get().FocusNodeInNewTab);
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Rename, NAME_None, LOCTEXT("Rename", "Rename"), LOCTEXT("Rename_Tooltip", "Renames this Dialog Graph."));
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Cut);
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Copy);
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Duplicate);
			MenuBuilder.AddMenuEntry(FMyDialogCommands::Get().DeleteEntry);
		}
		MenuBuilder.EndSection();

		FAssetSchemaAction_DialogBuilderGraph* Graph = SelectionAsGraph();

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
void SMyDialog::BuildAddNewMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("AddNewItem", LOCTEXT("AddOperations", "Add New"));

	if (UQuestBuilderGraph* CurrentBlueprint = GetQuestBuilderGraph())
	{
		MenuBuilder.AddMenuEntry(FDialogBuilder_EditorCommands::Get().AddNewQuestGraph);
		
	}
	MenuBuilder.EndSection();
}
TSharedRef<SWidget> SMyDialog::CreateAddToSectionButton(int32 InSectionID, TWeakPtr<SWidget> WeakRowWidget, FText AddNewText, FName MetaDataTag)
{
	return
		SNew(SButton)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.OnClicked(this, &SMyDialog::OnAddButtonClickedOnSection, InSectionID)
		.IsEnabled(this, &SMyDialog::CanAddNewElementToSection, InSectionID)
		.ContentPadding(FMargin(1, 0))
		.AddMetaData<FTagMetaData>(FTagMetaData(MetaDataTag))
		.ToolTipText(AddNewText)
		[
			SNew(SImage)
				.Image(FAppStyle::Get().GetBrush("Icons.PlusCircle"))
				.ColorAndOpacity(FSlateColor::UseForeground())
		];
}
void SMyDialog::OnCategoryNameCommitted(const FText& InNewText, ETextCommit::Type InTextCommit, TWeakPtr<struct FGraphActionNode> InAction)
{
	// Remove excess whitespace and prevent categories with just spaces
	FText CategoryName = FText::TrimPrecedingAndTrailing(InNewText);

	TArray<TSharedPtr<FEdGraphSchemaAction>> Actions;
	GraphActionMenu->GetCategorySubActions(InAction, Actions);

	if (Actions.Num())
	{
		const FScopedTransaction Transaction(LOCTEXT("RenameCategory", "Rename Category"));

		GetQuestBuilderGraph()->Modify();

		Refresh();
		SelectItemByName(FName(*CategoryName.ToString()), ESelectInfo::OnMouseClick, InAction.Pin()->SectionID, true);
	}
}
bool SMyDialog::CanRequestRenameOnActionNode(TWeakPtr<struct FGraphActionNode> InSelectedNode) const
{
	return true;
}

FText SMyDialog::OnGetSectionTitle(int32 InSectionID)
{
	FText SeperatorTitle;
	/* Setup an appropriate name for the section for this node */
	switch (InSectionID)
	{
	case DialogSectionID::DIALOGGRAPH:
		SeperatorTitle = NSLOCTEXT("GraphActionNode", "QuestGraphs", "QuestGraphs");
		break; 
	default:
	case DialogSectionID::NONE:
		SeperatorTitle = FText::GetEmpty();
		break;
	}
	
	return SeperatorTitle;
}
TSharedRef<SWidget> SMyDialog::OnGetSectionWidget(TSharedRef<SWidget> RowWidget, int32 InSectionID)
{
	TWeakPtr<SWidget> WeakRowWidget = RowWidget;

	FText AddNewText;
	FName MetaDataTag;

	switch (InSectionID)
	{
	case DialogSectionID::DIALOGGRAPH:
		AddNewText = LOCTEXT("AddNewQuestGraph", "New Dialog Graph");
		MetaDataTag = TEXT("AddNewGraph");
		break;
	default:
		return SNullWidget::NullWidget;
	}

	return CreateAddToSectionButton(InSectionID, WeakRowWidget, AddNewText, MetaDataTag);
}
FReply SMyDialog::OnAddButtonClickedOnSection(int32 InSectionID)
{
	TSharedPtr<FDialogBuilderEditor> QuestEditor = QuestEditorPtr.Pin();

	switch (InSectionID)
	{
	case DialogSectionID::DIALOGGRAPH:
		CommandList->ExecuteAction(FDialogBuilder_EditorCommands::Get().AddNewQuestGraph.ToSharedRef());
		break;
	}

	return FReply::Handled();
}
bool SMyDialog::CanAddNewElementToSection(int32 InSectionID) const
{
	return true;
}
bool SMyDialog::HandleActionMatchesName(FEdGraphSchemaAction* InAction, const FName& InName) const
{
	return false;
}
void SMyDialog::OnOpenGraph()
{
	OpenGraph(FDocumentTracker::OpenNewDocument);
}
void SMyDialog::OnOpenGraphInNewTab()
{
	OpenGraph(FDocumentTracker::ForceOpenNewDocument);
}
void SMyDialog::OnOpenExternalGraph()
{
}
bool SMyDialog::CanOpenGraph() const
{
	return true;
}
bool SMyDialog::CanOpenExternalGraph() const
{
	return false;
}
bool SMyDialog::CanFocusOnNode() const
{
	return false;
}
void SMyDialog::OnFocusNode()
{
}
void SMyDialog::OnFocusNodeInNewTab()
{
}
void SMyDialog::OnDeleteEntry()
{
	if (FAssetSchemaAction_DialogBuilderGraph* GraphAction = SelectionAsGraph())
	{
		OnDeleteGraph(GraphAction->EdGraph);
	}

	Refresh();
	//BlueprintEditorPtr.Pin()->GetInspector()->ShowDetailsForObjects(TArray<UObject*>());

}

bool SMyDialog::CanDeleteEntry() const
{
	// Cannot delete entries while not in editing mode
	/*if (!IsEditingMode())
	{
		return false;
	}*/

	if (FAssetSchemaAction_DialogBuilderGraph* GraphAction = SelectionAsGraph())
	{
		return (GraphAction->EdGraph ? GraphAction->EdGraph->bAllowDeletion : false);
	}
	return false;
}

bool SMyDialog::CanRequestRenameOnActionNode() const
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

bool SMyDialog::IsDuplicateActionVisible() const
{
	return false;
}
bool SMyDialog::CanDuplicateAction() const
{
	return false;
}
void SMyDialog::OnDuplicateAction()
{
}



void SMyDialog::OnCopy()
{
	
}
bool SMyDialog::CanCopy() const
{
	return false;
}
void SMyDialog::OnCut()
{
}
bool SMyDialog::CanCut() const
{
	return false;
}

void SMyDialog::OnDeleteGraph(UEdGraph* InGraph)
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
		GetQuestBuilderGraph()->Modify();

		InGraph->Modify();

		
		// Remove any  nodes bound to this graph
		TArray<UDialogBuilderEdNode*> AllNodes;
		FDialogBuilderEditorUtils::GetAllNodesOfClass<UDialogBuilderEdNode>(GetQuestBuilderGraph(), AllNodes);

		const bool bDontRecompile = true;
		for (UDialogBuilderEdNode* CompNode : AllNodes)
		{
			if (CompNode->GetQuestBuilderEdGraph() == InGraph)
			{
				FDialogBuilderEditorUtils::RemoveNode(GetQuestBuilderGraph(), CompNode, bDontRecompile);
			}
		}
		

		FDialogBuilderEditorUtils::RemoveGraph(GetQuestBuilderGraph(), InGraph);
		QuestEditorPtr.Pin()->CloseDocumentTab(InGraph);


		InGraph = NULL;
	}
}

void SMyDialog::OpenGraph(FDocumentTracker::EOpenDocumentCause InCause, bool bOpenExternalGraphInNewEditor)
{
	UEdGraph* GraphToOpen = nullptr;

	if (FAssetSchemaAction_DialogBuilderGraph* GraphAction = SelectionAsGraph())
	{
		GraphToOpen = GraphAction->EdGraph;
		// If we have no graph then this is an interface event, so focus on the event graph
		/*if (!GraphToOpen)
		{
			GraphToOpen = FDialogBuilderEditorUtils::FindQuestGraph(GetQuestBuilderGraph());
		}*/
	}

	if (GraphToOpen)
	{
		if (bOpenExternalGraphInNewEditor )
		{
			QuestEditorPtr.Pin()->JumpToHyperlink(GraphToOpen, false);
		}
		else
		{
			QuestEditorPtr.Pin()->OpenDocument(GraphToOpen, InCause);
		}
	}
}

bool SMyDialog::SelectionHasContextMenu() const
{
	TArray<TSharedPtr<FEdGraphSchemaAction> > SelectedActions;
	GraphActionMenu->GetSelectedActions(SelectedActions);
	return SelectedActions.Num() > 0;
}

void SMyDialog::OnObjectPropertyChanged(UObject* InObject, FPropertyChangedEvent& InPropertyChangedEvent)
{
	if (InObject == QuestBuilderGraph && (InPropertyChangedEvent.ChangeType != EPropertyChangeType::ValueSet && InPropertyChangedEvent.ChangeType != EPropertyChangeType::ArrayClear))
	{
		bNeedsRefresh = true;
	}
}

void SMyDialog::OnFilterTextChanged(const FText& InFilterText)
{
}
FText SMyDialog::GetFilterText() const
{
	return FText();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION



#undef LOCTEXT_NAMESPACE