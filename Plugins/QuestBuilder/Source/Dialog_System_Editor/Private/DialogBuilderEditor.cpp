// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderEditor.h"
#include "QuestBuilderNode_Root.h"
#include "Decorator/OrionDecorator.h"
#include "EngineGlobals.h"
#include "Editor/EditorEngine.h"
#include "Editor.h"
#include "UnrealEdGlobals.h"
#include "Event/OrionEvent.h"
#include "BlueprintEditor.h"
#include "UObject/ObjectSaveContext.h"
#include "GraphEditorActions.h"
#include "Framework/Commands/GenericCommands.h"
#include "DialogBuilderEditorToolbar.h"
#include "DialogBuilder_EditorCommands.h"
#include "DialogBuilderEdGraph.h"
#include "DialogBuilderEdNode.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "K2Node.h"
#include "GraphEditAction.h"
#include "DialogBuilderEdNode_Edge.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "HAL/PlatformApplicationMisc.h"
#include "EdGraphSchema_DialogBuilder.h"
#include "DialogBuilderEditorUtils.h"
#include "DialogBuilderFactory.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "SMyDialog.h"
#include "Task/QuestTask.h"
#include "Dialog_System_Editor.h"
#include "ContentBrowserModule.h"
#include "ContentBrowserFrontEndFilterExtension.h"
#include "Kismet2/KismetDebugUtilities.h"
#include "WorkflowOrientedApp/WorkflowUObjectDocuments.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include <Kismet2/KismetEditorUtilities.h>

#define LOCTEXT_NAMESPACE "QuestBuilderEditor"

const FName FDialogBuilderEditorTabs::MyQuestDetailID(TEXT("MyQuestDetail"));
const FName FDialogBuilderEditorTabs::QuestBuilderPropertyID(TEXT("QuestBuilderProperty"));
const FName FDialogBuilderEditorTabs::ViewportID(TEXT("Viewport"));
const FName FDialogBuilderEditorTabs::QuestBuilderEditorSettingsID(TEXT("QuestBuilderEditorSettings"));

const FName QuestBuilderEditorAppName = FName(TEXT("QuestBuilderEditorApp"));

//////////////////////////////////////////////////////////////////////////
FDialogBuilderEditor::FDialogBuilderEditor()
{
	EditingQuestGraph = nullptr;

#if ENGINE_MAJOR_VERSION < 5
	OnPackageSavedDelegateHandle = UPackage::PackageSavedEvent.AddRaw(this, &FDialogBuilderEditor::OnPackageSaved);
#else // #if ENGINE_MAJOR_VERSION < 5
	OnPackageSavedDelegateHandle = UPackage::PackageSavedWithContextEvent.AddRaw(this, &FDialogBuilderEditor::OnPackageSavedWithContext);
#endif // #else // #if ENGINE_MAJOR_VERSION < 5
}

FDialogBuilderEditor::~FDialogBuilderEditor()
{
#if ENGINE_MAJOR_VERSION < 5
	UPackage::PackageSavedEvent.Remove(OnPackageSavedDelegateHandle);
#else // #if ENGINE_MAJOR_VERSION < 5
	UPackage::PackageSavedWithContextEvent.Remove(OnPackageSavedDelegateHandle);
#endif // #else // #if ENGINE_MAJOR_VERSION < 5
}

void FDialogBuilderEditor::Initialize(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UObject* InObject)
{
	UQuestBuilderGraph* QuestGraphToEdit = Cast<UQuestBuilderGraph>(InObject);

	if (QuestGraphToEdit != nullptr)
	{
		EditingQuestGraph = QuestGraphToEdit;
	}

	//Binding Functionality
	EditingQuestGraph->GetOutermost()->PackageMarkedDirtyEvent.AddRaw(this, &FDialogBuilderEditor::OnPackageMarkedDirty);

	TSharedPtr<FDialogBuilderEditor> ThisPtr(SharedThis(this));
	if (!DocumentManager.IsValid())
	{
		DocumentManager = MakeShareable(new FDocumentTracker);
		DocumentManager->Initialize(ThisPtr);

		// Register the document factories
		{
			TSharedRef<FDocumentTabFactory> GraphEditorFactory = MakeShareable(new FDialogGraphEditorSummoner(ThisPtr,
				FDialogGraphEditorSummoner::FOnCreateGraphEditorWidget::CreateSP(this, &FDialogBuilderEditor::CreateGraphEditorWidget)
			));

			// Also store off a reference to the grapheditor factory so we can find all the tabs spawned by it later.
			QuestEditorTabFactoryPtr = GraphEditorFactory;
			DocumentManager->RegisterDocumentFactory(GraphEditorFactory);
		}
	}

	TArray<UObject*> ObjectsToEdit;
	if (EditingQuestGraph != nullptr)
	{
		ObjectsToEdit.Add(EditingQuestGraph);
	}

	//Make Toolbar
	if (!ToolbarBuilder.IsValid())
	{
		ToolbarBuilder = MakeShareable(new FDialogBuilderEditorToolbar(SharedThis(this)));
	}

	// if we are already editing objects, dont try to recreate the editor from scratch but update the list of objects in edition
	// ex: BehaviorTree may want to reuse an editor already opened for its associated Blackboard asset.
	const TArray<UObject*>* EditedObjects = GetObjectsCurrentlyBeingEdited();
	if (EditedObjects == nullptr || EditedObjects->Num() == 0)
	{
		FGenericCommands::Register();
		FGraphEditorCommands::Register();
		FMyDialogCommands::Register();
		FDialogBuilder_EditorCommands::Register();

		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

		ToolbarBuilder->AddQuestSystemToolbar(ToolbarExtender);

		BindCommands();
		CreateInternalWidgets();

		// Layout
		const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_QuestBuilderEditor_Layout_v1")
			->AddArea
			(
				FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
#if ENGINE_MAJOR_VERSION < 5
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.1f)
					->AddTab(GetToolbarTabId(), ETabState::OpenedTab)->SetHideTabWell(true)
				)
#endif // #if ENGINE_MAJOR_VERSION < 5
				->Split
				(
					FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)->SetSizeCoefficient(0.9f)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.2f)
						->AddTab(FDialogBuilderEditorTabs::MyQuestDetailID, ETabState::OpenedTab)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.55f)
						->AddTab("Document", ETabState::ClosedTab)
					)
					->Split
					(
						FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)->SetSizeCoefficient(0.25f)
						->Split
						(
							FTabManager::NewStack()
							->SetSizeCoefficient(0.55f)
							->AddTab(FDialogBuilderEditorTabs::QuestBuilderPropertyID, ETabState::OpenedTab)
						)
					)
				)
			);

		const bool bCreateDefaultStandaloneMenu = true;
		const bool bCreateDefaultToolbar = true;
		InitAssetEditor(Mode, InitToolkitHost, QuestBuilderEditorAppName, StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, EditingQuestGraph, false);


		if (EditingQuestGraph && EditingQuestGraph->bIsNewlyCreated)
		{
			NewDocument_OnClicked(CGT_NewQuestGraph);
			EditingQuestGraph->bIsNewlyCreated = false;
			EditingQuestGraph->Modify();
		}
		else
		{
			if (GetQuestBuilderGraph()->QuestGraphPages.Num() > 0)
			{
				OpenDocument(GetQuestBuilderGraph()->QuestGraphPages[0], FDocumentTracker::OpenNewDocument);
				
				//bind on graph changes
				for (UEdGraph* EdGraph : EditingQuestGraph->QuestGraphPages)
				{
					if (EdGraph)
					{
						if (UDialogBuilderEdGraph* QuestGraph = Cast<UDialogBuilderEdGraph>(EdGraph))
						{
							QuestGraph->AddOnGraphChangedHandler(FOnGraphChanged::FDelegate::CreateSP(this, &FDialogBuilderEditor::OnGraphChanged));
						}
					}
				}
			}
		}
	}
	else
	{
		for (UObject* ObjectToEdit : ObjectsToEdit)
		{
			if (!EditedObjects->Contains(ObjectToEdit))
			{
				AddEditingObject(ObjectToEdit);
			}
		}
	}


	RegenerateMenusAndToolbars();
	RebuildQuestBuilderGraphPages();
}


void FDialogBuilderEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	DocumentManager->SetTabManager(InTabManager);

	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_QuestBuilderEditor", "Generic Graph Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FDialogBuilderEditorTabs::MyQuestDetailID, FOnSpawnTab::CreateSP(this, &FDialogBuilderEditor::SpawnTab_MyQuest))
		.SetDisplayName(LOCTEXT("Dialog Details", "My Dialog"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FDialogBuilderEditorTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FDialogBuilderEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("GraphCanvasTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.EventGraph_16x"));

	InTabManager->RegisterTabSpawner(FDialogBuilderEditorTabs::QuestBuilderPropertyID, FOnSpawnTab::CreateSP(this, &FDialogBuilderEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab", "Property"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FDialogBuilderEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FDialogBuilderEditorTabs::MyQuestDetailID);
	InTabManager->UnregisterTabSpawner(FDialogBuilderEditorTabs::ViewportID);
	InTabManager->UnregisterTabSpawner(FDialogBuilderEditorTabs::QuestBuilderPropertyID);
	InTabManager->UnregisterTabSpawner(FDialogBuilderEditorTabs::QuestBuilderEditorSettingsID);
}





void FDialogBuilderEditor::BindCommands()
{
	ToolkitCommands->MapAction(FDialogBuilder_EditorCommands::Get().NewTask,
		FExecuteAction::CreateSP(this, &FDialogBuilderEditor::CreateNewTask),
		FCanExecuteAction::CreateSP(this, &FDialogBuilderEditor::CanCreateNewTask),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateSP(this, &FDialogBuilderEditor::IsNewTaskButtonVisible)
	);

	ToolkitCommands->MapAction(FDialogBuilder_EditorCommands::Get().NewQuestCondition,
		FExecuteAction::CreateSP(this, &FDialogBuilderEditor::CreateNewQuestCondition),
		FCanExecuteAction::CreateSP(this, &FDialogBuilderEditor::CanCreateQuestCondition)
	);

	ToolkitCommands->MapAction(FDialogBuilder_EditorCommands::Get().NewQuestEvent,
		FExecuteAction::CreateSP(this, &FDialogBuilderEditor::CreateNewQuestEvent),
		FCanExecuteAction::CreateSP(this, &FDialogBuilderEditor::CanCreateQuestEvent)
	); 
	
	ToolkitCommands->MapAction(FDialogBuilder_EditorCommands::Get().QuestSetting,
		FExecuteAction::CreateSP(this, &FDialogBuilderEditor::OpenQuestSetting),
		FCanExecuteAction::CreateSP(this, &FDialogBuilderEditor::CanOpenQuestSetting)
	);
	
	ToolkitCommands->MapAction(FDialogBuilder_EditorCommands::Get().AddNewQuestGraph,
		FExecuteAction::CreateSP(this, &FDialogBuilderEditor::NewDocument_OnClicked, CGT_NewQuestGraph),
		FCanExecuteAction::CreateSP(this, &FDialogBuilderEditor::InEditingMode),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateSP(this, &FDialogBuilderEditor::NewDocument_IsVisibleForType, CGT_NewQuestGraph)
	);

}

void FDialogBuilderEditor::CreateCommandList()
{
	if (GraphEditorCommands.IsValid()) {
		return;
	}

	GraphEditorCommands = MakeShareable(new FUICommandList);
	// Can't use CreateSP here because derived editor are already implementing TSharedFromThis<FAssetEditorToolkit>
	// however it should be safe, since commands are being used only within this editor
	// if it ever crashes, this function will have to go away and be reimplemented in each derived class

	GraphEditorCommands->MapAction(FDialogBuilder_EditorCommands::Get().AddNewQuestGraph,
		FExecuteAction::CreateSP(this, &FDialogBuilderEditor::NewDocument_OnClicked, CGT_NewQuestGraph),
		FCanExecuteAction::CreateSP(this, &FDialogBuilderEditor::InEditingMode),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateSP(this, &FDialogBuilderEditor::NewDocument_IsVisibleForType, CGT_NewQuestGraph)
	);

	GraphEditorCommands->MapAction(FDialogBuilder_EditorCommands::Get().NewTask,
		FExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CreateNewTask),
		FCanExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CanCreateNewTask));

	GraphEditorCommands->MapAction(FDialogBuilder_EditorCommands::Get().NewQuestCondition,
		FExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CreateNewQuestEvent),
		FCanExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CanCreateQuestEvent));
	
	GraphEditorCommands->MapAction(FDialogBuilder_EditorCommands::Get().NewQuestEvent,
		FExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CreateNewQuestCondition),
		FCanExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CanCreateQuestCondition));

	GraphEditorCommands->MapAction(FDialogBuilder_EditorCommands::Get().QuestSetting,
		FExecuteAction::CreateRaw(this, &FDialogBuilderEditor::OpenQuestSetting),
		FCanExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CanOpenQuestSetting));


	GraphEditorCommands->MapAction(FGenericCommands::Get().SelectAll,
		FExecuteAction::CreateRaw(this, &FDialogBuilderEditor::SelectAllNodes),
		FCanExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CanSelectAllNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Delete,
		FExecuteAction::CreateRaw(this, &FDialogBuilderEditor::DeleteSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CanDeleteNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Copy,
		FExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CopySelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CanCopyNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Cut,
		FExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CutSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CanCutNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Paste,
		FExecuteAction::CreateRaw(this, &FDialogBuilderEditor::PasteNodes),
		FCanExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CanPasteNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Duplicate,
		FExecuteAction::CreateRaw(this, &FDialogBuilderEditor::DuplicateNodes),
		FCanExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CanDuplicateNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Rename,
		FExecuteAction::CreateSP(this, &FDialogBuilderEditor::OnRenameNode),
		FCanExecuteAction::CreateSP(this, &FDialogBuilderEditor::CanRenameNodes)
	);
}

TSharedPtr<SGraphEditor> FDialogBuilderEditor::GetCurrGraphEditor() const
{
	return ViewportWidget;
}

FGraphPanelSelectionSet FDialogBuilderEditor::GetSelectedNodes() const
{
	FGraphPanelSelectionSet CurrentSelection;
	TSharedPtr<SGraphEditor> FocusedGraphEd = GetCurrGraphEditor();
	if (FocusedGraphEd.IsValid())
	{
		CurrentSelection = FocusedGraphEd->GetSelectedNodes();
	}

	return CurrentSelection;
}

FName FDialogBuilderEditor::GetToolkitFName() const
{
	return FName("FQuestGraphEditor");
}

FText FDialogBuilderEditor::GetBaseToolkitName() const
{
	return LOCTEXT("QuestGraphEditorAppLabel", "Dialog Graph Editor");
}


FText FDialogBuilderEditor::GetToolkitName() const
{
	const bool bDirtyState = EditingQuestGraph->GetOutermost()->IsDirty();

	FFormatNamedArguments Args;
	Args.Add(TEXT("QuestGraphName"), FText::FromString(EditingQuestGraph->GetName()));
	Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
	return FText::Format(LOCTEXT("QuestGraphEditorToolkitName", "{QuestGraphName}{DirtyState}"), Args);
}

FText FDialogBuilderEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(EditingQuestGraph);
}

FLinearColor FDialogBuilderEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::Blue;
}

FString FDialogBuilderEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("QuestGraphEditor");
}

FString FDialogBuilderEditor::GetDocumentationLink() const
{
	//make documentation from notion add this link
	return TEXT("");
}

void FDialogBuilderEditor::SaveAsset_Execute()
{
	FAssetEditorToolkit::SaveAsset_Execute();

}

void FDialogBuilderEditor::RefreshEditors()
{
	
}

void FDialogBuilderEditor::RefreshMyQuest()
{
}

void FDialogBuilderEditor::RefreshInspector()
{
}

void FDialogBuilderEditor::AddToSelection(UEdGraphNode* InNode)
{
}

void FDialogBuilderEditor::JumpToHyperlink(const UObject* ObjectReference, bool bRequestRename)
{
	//SetCurrentMode(FBlueprintEditorApplicationModes::StandardBlueprintEditorMode);
	if (const UEdGraph* Graph = Cast<const UEdGraph>(ObjectReference))
	{
		// Navigating into things should re-use the current tab when it makes sense
		FDocumentTracker::EOpenDocumentCause OpenMode = FDocumentTracker::OpenNewDocument;
		if ((Graph->GetSchema()->GetGraphType(Graph) == GT_Ubergraph) /*|| Cast<UK2Node>(Graph->GetOuter())*/ || Cast<UEdGraph>(Graph-> GetOuter()))
		{
			// Ubergraphs directly reuse the current graph
			OpenMode = FDocumentTracker::NavigatingCurrentDocument;
		}
		else
		{
			// Walk up the outer chain to see if any tabs have a parent of this document open for edit, and if so
			// we should reuse that one and drill in deeper instead
			for (UObject* WalkPtr = const_cast<UEdGraph*>(Graph); WalkPtr != nullptr; WalkPtr = WalkPtr->GetOuter())
			{
				TArray< TSharedPtr<SDockTab> > TabResults;
				if (FindOpenTabsContainingDocument(WalkPtr, /*out*/ TabResults))
				{
					// See if the parent was active
					bool bIsActive = false;
					for (TSharedPtr<SDockTab> Tab : TabResults)
					{
						if (Tab->IsActive())
						{
							bIsActive = true;
							break;
						}
					}

					if (bIsActive)
					{
						OpenMode = FDocumentTracker::NavigatingCurrentDocument;
						break;
					}
				}
			}
		}

		// Force it to open in a new document if shift is pressed
		const bool bIsShiftPressed = FSlateApplication::Get().GetModifierKeys().IsShiftDown();
		if (bIsShiftPressed)
		{
			OpenMode = FDocumentTracker::ForceOpenNewDocument;
		}

		// Open the document
		OpenDocument(Graph, OpenMode);
	}

	else
	{
		UE_LOG(LogBlueprint, Warning, TEXT("Unknown type of hyperlinked object (%s), cannot focus it"), *GetNameSafe(ObjectReference));
	}

	//@TODO: Hacky way to ensure a message is seen when hitting an exception and doing intraframe debugging
	const FText ExceptionMessage = FKismetDebugUtilities::GetAndClearLastExceptionMessage();
	if (!ExceptionMessage.IsEmpty())
	{
		LogSimpleMessage(ExceptionMessage);
	}
}

void FDialogBuilderEditor::JumpToPin(const UEdGraphPin* Pin)
{
}

void FDialogBuilderEditor::SummonSearchUI(bool bSetFindWithinBlueprint, FString NewSearchTerms, bool bSelectFirstResult)
{
}

void FDialogBuilderEditor::SummonFindAndReplaceUI()
{
}

TSharedPtr<SGraphEditor> FDialogBuilderEditor::OpenGraphAndBringToFront(UEdGraph* Graph, bool bSetFocus)
{
	return TSharedPtr<SGraphEditor>();
}

void FDialogBuilderEditor::UpdateToolbar()
{
}

void FDialogBuilderEditor::RegisterToolbarTab(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
}

void FDialogBuilderEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	/*if (GetObjectsCurrentlyBeingEdited()->Num() > 0)
	{
		TArray<UObject*>& LocalEditingObjects = const_cast<TArray<UObject*>&>(GetEditingObjects());

		Collector.AddReferencedObjects(LocalEditingObjects);
	}

	Collector.AddReferencedObject(EditingQuestGraph);*/
}

FString FDialogBuilderEditor::GetReferencerName() const
{
	return FString();
}

UQuestBuilderGraph* FDialogBuilderEditor::GetQuestBuilderGraph() const
{
	return EditingQuestGraph;
}

bool FDialogBuilderEditor::NewDocument_IsVisibleForType(ECreatedQuestDocumentType GraphType) const
{
	return true;
}

void FDialogBuilderEditor::NewDocument_OnClicked(ECreatedQuestDocumentType GraphType)
{
	FText DocumentNameText;
	bool bResetMyBlueprintFilter = false;

	switch (GraphType)
	{
	case CGT_NewQuestGraph:
		DocumentNameText = LOCTEXT("NewDocQuestGraphName", "QuestGraph");
		bResetMyBlueprintFilter = true;
		break;
	
	default:
		DocumentNameText = LOCTEXT("NewDocNewName", "NewDocument");
		break;
	}

	FName DocumentName = FName(*DocumentNameText.ToString());

	

	// Make sure the new name is valid
	DocumentName = FDialogBuilderEditorUtils::FindUniqueQuestName(DocumentNameText.ToString());
		
	//check(IsEditingSingleBlueprint());

	const FScopedTransaction Transaction(LOCTEXT("AddNewQuestGraph", "Add New Dialog Graph"));
	GetQuestBuilderGraph()->Modify();

	UEdGraph* NewGraph = nullptr;

	
	if (GraphType == CGT_NewQuestGraph)
	{
		NewGraph = FDialogBuilderEditorUtils::CreateNewGraph(GetQuestBuilderGraph(), DocumentName, UDialogBuilderEdGraph::StaticClass(), UEdGraphSchema_DialogBuilder::StaticClass());
		NewGraph->AddOnGraphChangedHandler(FOnGraphChanged::FDelegate::CreateSP(this, &FDialogBuilderEditor::OnGraphChanged));
		FDialogBuilderEditorUtils::AddQuestGraphPage(GetQuestBuilderGraph(), NewGraph);

	}
	else
	{
		ensureMsgf(false, TEXT("GraphType is invalid"));
	}

	// Now open the new graph
	if (NewGraph)
	{
		OpenDocument(NewGraph, FDocumentTracker::OpenNewDocument);

		RenameNewlyAddedAction(DocumentName);
	}
	else
	{
		LogSimpleMessage(LOCTEXT("AddDocument_Error", "Adding new document failed."));
	}
}

bool FDialogBuilderEditor::InEditingMode() const
{
	return true;
}

TSharedPtr<SDockTab> FDialogBuilderEditor::OpenDocument(const UObject* DocumentID, FDocumentTracker::EOpenDocumentCause Cause)
{
	TSharedRef<FTabPayload_UObject> Payload = FTabPayload_UObject::Make(DocumentID);
	return DocumentManager->OpenDocument(Payload, Cause);
}

void FDialogBuilderEditor::CloseDocumentTab(const UObject* DocumentID)
{
	TSharedRef<FTabPayload_UObject> Payload = FTabPayload_UObject::Make(DocumentID);
	DocumentManager->CloseTab(Payload);
}

void FDialogBuilderEditor::RenameNewlyAddedAction(FName InActionName)
{
	if (MyQuestWidget.IsValid())
	{
		// Force a refresh immediately, the item has to be present in the list for the rename requests to be successful.
		MyQuestWidget->Refresh();
		MyQuestWidget->SelectItemByName(InActionName, ESelectInfo::OnMouseClick);
		MyQuestWidget->OnRequestRenameOnActionNode();
	}
}

void FDialogBuilderEditor::LogSimpleMessage(const FText& MessageText)
{
	FNotificationInfo Info(MessageText);
	Info.ExpireDuration = 3.0f;
	Info.bUseLargeFont = false;
	TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
	if (Notification.IsValid())
	{
		Notification->SetCompletionState(SNotificationItem::CS_Fail);
	}
}

TSharedRef<SWidget> FDialogBuilderEditor::CreateGraphTitleBarWidget(TSharedRef<FTabInfo> InTabInfo, UEdGraph* InGraph)
{
   // Create a horizontal box to serve as the title bar
	return SNew(SBox);
		
		//SNew(SHorizontalBox)
		//+ SHorizontalBox::Slot()
		//.FillWidth(1.0f)
		//.VAlign(VAlign_Center)
		//.Padding(FMargin(5.0f, 0.0f))
		//[
		//	// Add the graph name as a text block
		//	SNew(STextBlock)
		//		.Text(FText::FromString(InGraph->GetName()))
		//		.TextStyle(FAppStyle::Get(), "GraphBreadcrumbButtonText")
		//];
}


const FSlateBrush* FDialogBuilderEditor::GetGlyphForGraph(const UEdGraph* Graph, bool bInLargeIcon)
{
	const FSlateBrush* ReturnValue = FAppStyle::GetBrush(bInLargeIcon ? TEXT("GraphEditor.Function_24x") : TEXT("GraphEditor.Function_16x"));

	check(Graph != nullptr);
	const UEdGraphSchema* Schema = Graph->GetSchema();
	if (Schema != nullptr)
	{
		const EGraphType GraphType = Schema->GetGraphType(Graph);
		switch (GraphType)
		{
		default:
		case GT_Ubergraph:
		{
			ReturnValue = FAppStyle::GetBrush(bInLargeIcon ? TEXT("GraphEditor.EventGraph_24x") : TEXT("GraphEditor.EventGraph_16x"));
		}
		break;
		}
	}

	return ReturnValue;
}

bool FDialogBuilderEditor::FindOpenTabsContainingDocument(const UObject* DocumentID, TArray<TSharedPtr<SDockTab>>& Results)
{
	int32 StartingCount = Results.Num();

	TSharedRef<FTabPayload_UObject> Payload = FTabPayload_UObject::Make(DocumentID);

	DocumentManager->FindMatchingTabs(Payload, /*inout*/ Results);

	// Did we add anything new?
	return (StartingCount != Results.Num());
}


void FDialogBuilderEditor::InitializeDocumentTab()
{
	check(IsEditingSingleQuestGraph());

	UQuestBuilderGraph* QuestBuilderGraph = GetQuestBuilderGraph();
	if (QuestBuilderGraph->LastEditedDocuments.Num() == 0)
	{
			QuestBuilderGraph->LastEditedDocuments.Add(FDialogBuilderEditorUtils::FindQuestGraph(QuestBuilderGraph));
	}

	for (int32 i = 0; i < QuestBuilderGraph->LastEditedDocuments.Num(); i++)
	{
		if (UObject* Obj = QuestBuilderGraph->LastEditedDocuments[i].EditedObjectPath.ResolveObject())
		{
			if (UEdGraph* Graph = Cast<UEdGraph>(Obj))
			{
				struct LocalStruct
				{
					static TSharedPtr<SDockTab> OpenGraphTree(FDialogBuilderEditor* InQuestSystemGraphEditor, UEdGraph* InGraph)
					{
						FDocumentTracker::EOpenDocumentCause OpenCause = FDocumentTracker::QuickNavigateCurrentDocument;

						for (UObject* OuterObject = InGraph->GetOuter(); OuterObject; OuterObject = OuterObject->GetOuter())
						{
							if (OuterObject->IsA<UQuestBuilderGraph>())
							{
								// reached up to the QuestBuilderGraph for the graph, we are done climbing the tree
								OpenCause = FDocumentTracker::RestorePreviousDocument;
								break;
							}
							else if (UEdGraph* OuterGraph = Cast<UEdGraph>(OuterObject))
							{
								// Found another graph, open it up
								OpenGraphTree(InQuestSystemGraphEditor, OuterGraph);
								break;
							}
						}

						return InQuestSystemGraphEditor->OpenDocument(InGraph, OpenCause);
					}
				};
				TSharedPtr<SDockTab> TabWithGraph = LocalStruct::OpenGraphTree(this, Graph);
				if (TabWithGraph.IsValid())
				{
					TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(TabWithGraph->GetContent());
					GraphEditor->SetViewLocation(QuestBuilderGraph->LastEditedDocuments[i].SavedViewOffset, QuestBuilderGraph->LastEditedDocuments[i].SavedZoomAmount);
				}
			}
			else
			{
				TSharedPtr<SDockTab> TabWithGraph = OpenDocument(Obj, FDocumentTracker::RestorePreviousDocument);
			}
		}
	}
}



bool FDialogBuilderEditor::IsEditingSingleQuestGraph() const
{
	return GetQuestBuilderGraph() != nullptr;
}

UEdGraph* FDialogBuilderEditor::GetFocusedGraph() const
{
	if (GetCurrGraphEditor().IsValid())
	{
		if (UEdGraph* Graph = GetCurrGraphEditor()->GetCurrentGraph())
		{
			if (IsValid(Graph))
			{
				return Graph;
			}
		}
	}
	return nullptr;
}

UEdGraphNode* FDialogBuilderEditor::GetSingleSelectedNode() const
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	return (SelectedNodes.Num() == 1) ? Cast<UEdGraphNode>(*SelectedNodes.CreateConstIterator()) : nullptr;
}

void FDialogBuilderEditor::OnGraphEditorFocused(const TSharedRef<class SGraphEditor>& InGraphEditor)
{
	// Update the graph editor that is currently focused
	ViewportWidget = InGraphEditor;
	InGraphEditor->SetPinVisibility(SGraphEditor::EPinVisibility::Pin_Show);

	// Update the inspector as well, to show selection from the focused graph editor
	FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	//FocusInspectorOnGraphSelection(SelectedNodes, /*bForceRefresh=*/ true);

	// During undo, garbage graphs can be temporarily brought into focus, ensure that before a refresh of the MyBlueprint window that the graph is owned by a Blueprint
	if (ViewportWidget.IsValid() && MyQuestWidget.IsValid())
	{
		// The focused graph can be garbage as well
		TWeakObjectPtr< UEdGraph > FocusedGraphPtr = ViewportWidget->GetCurrentGraph();
		UEdGraph* FocusedGraph = FocusedGraphPtr.Get();
		
		if (FocusedGraph != nullptr)
		{
			if (UDialogBuilderEdGraph* QuestEdGraph = Cast<UDialogBuilderEdGraph>(FocusedGraph))
			{
				QuestEdGraph->SEditorGraph = ViewportWidget.Get();
			}
			MyQuestWidget->Refresh();
		}
	}

	
}

void FDialogBuilderEditor::OnGraphEditorBackgrounded(const TSharedRef<SGraphEditor>& InGraphEditor)
{
}

bool FDialogBuilderEditor::IsGraphInCurrentQuestGraph(const UEdGraph* InGraph) const
{
	bool bEditable = true;

	UQuestBuilderGraph* EditingBP = GetQuestBuilderGraph();
	if (EditingBP)
	{
		TArray<UEdGraph*> Graphs;
		EditingBP->GetAllGraphs(Graphs);
		bEditable &= Graphs.Contains(InGraph);
	}

	return bEditable;
}

FGraphAppearanceInfo FDialogBuilderEditor::GetGraphAppearance() const
{
	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText", "Dialog Editor");

	if (FDialogBuilderEditor::IsPIESimulating())
	{
		if (GetQuestBuilderGraph()->QuestComponent)
		{
			AppearanceInfo.PIENotifyText = LOCTEXT("ActiveLabel", "ACTIVE");
		}
		else
		{
			AppearanceInfo.PIENotifyText = LOCTEXT("InactiveLabel", "INACTIVE");
		}
	}
	
	
	return AppearanceInfo;
}

bool FDialogBuilderEditor::InEditingMode(bool bGraphIsEditable) const
{
	return bGraphIsEditable && FDialogBuilderEditor::IsPIENotSimulating();
}


bool FDialogBuilderEditor::IsPIESimulating()
{
	return GEditor->bIsSimulatingInEditor || GEditor->PlayWorld;
}

bool FDialogBuilderEditor::IsPIENotSimulating()
{
	return !GEditor->bIsSimulatingInEditor && (GEditor->PlayWorld == NULL);
}

void FDialogBuilderEditor::OnChangeBreadCrumbGraph(UEdGraph* InGraph)
{
}

TSharedRef<SDockTab> FDialogBuilderEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FDialogBuilderEditorTabs::ViewportID);

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.Label(LOCTEXT("ViewportTab_Title", "Viewport"));

	if (ViewportWidget.IsValid())
	{
		SpawnedTab->SetContent(ViewportWidget.ToSharedRef());
	}

	return SpawnedTab;
}

TSharedRef<SDockTab> FDialogBuilderEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FDialogBuilderEditorTabs::QuestBuilderPropertyID);

	return SNew(SDockTab)
#if ENGINE_MAJOR_VERSION < 5
		.Icon(FAppStyle::GetBrush("LevelEditor.Tabs.Details"))
#endif // #if ENGINE_MAJOR_VERSION < 5
		.Label(LOCTEXT("Details_Title", "Property"))
		[
			PropertyWidget.ToSharedRef()
		];
}

TSharedRef<SDockTab> FDialogBuilderEditor::SpawnTab_EditorSettings(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FDialogBuilderEditorTabs::QuestBuilderEditorSettingsID);

	return SNew(SDockTab)
#if ENGINE_MAJOR_VERSION < 5
		.Icon(FAppStyle::GetBrush("LevelEditor.Tabs.Details"))
#endif // #if ENGINE_MAJOR_VERSION < 5
		.Label(LOCTEXT("EditorSettings_Title", "Generic Graph Editor Setttings"))
		[
			EditorSettingsWidget.ToSharedRef()
		];
}

TSharedRef<SDockTab> FDialogBuilderEditor::SpawnTab_MyQuest(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FDialogBuilderEditorTabs::MyQuestDetailID);

	return SNew(SDockTab)
#if ENGINE_MAJOR_VERSION < 5
		.Icon(FAppStyle::GetBrush("LevelEditor.Tabs.Details"))
#endif // #if ENGINE_MAJOR_VERSION < 5
		.Label(LOCTEXT("MyQuest_Title", "My Dialog"))
		[
			MyQuestWidget.ToSharedRef()
		];
}

void FDialogBuilderEditor::CreateInternalWidgets()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.NotifyHook = this;
	DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;

	PropertyWidget = PropertyModule.CreateDetailView(DetailsViewArgs);
	PropertyWidget->SetObject( NULL );
	PropertyWidget->OnFinishedChangingProperties().AddSP(this, &FDialogBuilderEditor::OnFinishedChangingProperties);


	this->MyQuestWidget = SNew(SMyDialog, SharedThis(this));
}

TSharedRef<SGraphEditor> FDialogBuilderEditor::CreateGraphEditorWidget(TSharedRef<class FTabInfo> InTabInfo, UEdGraph* InGraph)
{

	// Create the title bar widget
	TSharedPtr<SWidget> TitleBarWidget = CreateGraphTitleBarWidget(InTabInfo, InGraph);

	CreateCommandList();

	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FDialogBuilderEditor::OnSelectedNodesChanged);
	InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FDialogBuilderEditor::OnNodeDoubleClicked);

	// Make full graph editor
	const bool bGraphIsEditable = InGraph->bEditable;
	return SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.IsEditable(this, &FDialogBuilderEditor::InEditingMode, bGraphIsEditable)
		.TitleBar(TitleBarWidget)
		.Appearance(this, &FDialogBuilderEditor::GetGraphAppearance)
		.GraphToEdit(InGraph)
		.GraphEvents(InEvents)
		.AutoExpandActionMenu(true);
}


void FDialogBuilderEditor::RebuildQuestBuilderGraphPages()
{
	if (EditingQuestGraph == nullptr)
	{
		return;
	}

	GetQuestBuilderGraph()->NodeIDMap.Reset();
	for (UEdGraph* EdGraph : EditingQuestGraph->QuestGraphPages)
	{
		if (EdGraph)
		{
			if (UDialogBuilderEdGraph* QuestGraph = Cast<UDialogBuilderEdGraph>(EdGraph))
			{
				QuestGraph->RebuildQuestBuilderGraph();
			}
		}
	}
}

	



void FDialogBuilderEditor::OnGraphChanged(const FEdGraphEditAction& Action)
{
	UDialogBuilderEdGraph* QuestGraph = Cast<UDialogBuilderEdGraph>(Action.Graph);
	if (QuestGraph)
	{
		QuestGraph->CacheNodeIDMap();
	}
}
void FDialogBuilderEditor::SelectAllNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		CurrentGraphEditor->SelectAllNodes();
	}
}
bool FDialogBuilderEditor::CanSelectAllNodes()
{
	return true;
}
void FDialogBuilderEditor::DeleteSelectedNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return;
	}

	const FScopedTransaction Transaction(FGenericCommands::Get().Delete->GetDescription());

	CurrentGraphEditor->GetCurrentGraph()->Modify();

	const FGraphPanelSelectionSet SelectedNodes = CurrentGraphEditor->GetSelectedNodes();
	CurrentGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		UEdGraphNode* EdNode = Cast<UEdGraphNode>(*NodeIt);
		if (EdNode == nullptr || !EdNode->CanUserDeleteNode())
			continue;;

		if (UDialogBuilderEdNode* QuestEdNode = Cast<UDialogBuilderEdNode>(EdNode))
		{
			QuestEdNode->Modify();

			const UEdGraphSchema* Schema = QuestEdNode->GetSchema();
			if (Schema != nullptr)
			{
				Schema->BreakNodeLinks(*QuestEdNode);
			}

			QuestEdNode->DestroyNode();
		}
		else
		{
			EdNode->Modify();
			EdNode->DestroyNode();
		}

	}
}
bool FDialogBuilderEditor::CanDeleteNodes()
{
	if (IsPIESimulating())
		return false;

	// If any of the nodes can be deleted then we should allow deleting
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node != nullptr && Node->CanUserDeleteNode())
		{
			return true;
		}
	}
	return false;
}
void FDialogBuilderEditor::DeleteSelectedDuplicatableNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return;
	}

	const FGraphPanelSelectionSet OldSelectedNodes = CurrentGraphEditor->GetSelectedNodes();
	CurrentGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node && Node->CanDuplicateNode())
		{
			CurrentGraphEditor->SetNodeSelection(Node, true);
		}
	}

	// Delete the duplicatable nodes
	DeleteSelectedNodes();

	CurrentGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter))
		{
			CurrentGraphEditor->SetNodeSelection(Node, true);
		}
	}
}
void FDialogBuilderEditor::CutSelectedNodes()
{
	CopySelectedNodes();
	DeleteSelectedDuplicatableNodes();
	ShouldGetNewID = false;
}
bool FDialogBuilderEditor::CanCutNodes()
{
	if (IsPIESimulating())
		return false;
	return CanCopyNodes() && CanDeleteNodes();
}

void FDialogBuilderEditor::CopySelectedNodes()
{
	// Export the selected nodes and place the text on the clipboard
	FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	FString ExportedText;

	ShouldGetNewID = true;

	for (FGraphPanelSelectionSet::TIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node == nullptr)
		{
			SelectedIter.RemoveCurrent();
			continue;
		}

		if (UDialogBuilderEdNode_Edge* EdNode_Edge = Cast<UDialogBuilderEdNode_Edge>(*SelectedIter))
		{
			UDialogBuilderEdNode* StartNode = EdNode_Edge->GetStartNode();
			UDialogBuilderEdNode* EndNode = EdNode_Edge->GetEndNode();

			if (!SelectedNodes.Contains(StartNode) || !SelectedNodes.Contains(EndNode))
			{
				SelectedIter.RemoveCurrent();
				continue;
			}
		}

		Node->PrepareForCopying();
	}

	FEdGraphUtilities::ExportNodesToText(SelectedNodes, ExportedText);
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);
}
bool FDialogBuilderEditor::CanCopyNodes()
{
	// If any of the nodes can be duplicated then we should allow copying
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node && Node->CanDuplicateNode())
		{
			return true;
		}
	}

	return false;
}
void FDialogBuilderEditor::PasteNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		PasteNodesHere(CurrentGraphEditor->GetCurrentGraph(), CurrentGraphEditor->GetPasteLocation());
	}
}
void FDialogBuilderEditor::PasteNodesHere(UEdGraph* DestinationGraph, const FVector2D& Location)
{
	// Find the graph editor with focus
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return;
	}
	// Select the newly pasted stuff
	UEdGraph* EdGraph = DestinationGraph;
	{
		const FScopedTransaction Transaction(FGenericCommands::Get().Paste->GetDescription());
		EdGraph->Modify();

		// Clear the selection set (newly pasted stuff will be selected)
		CurrentGraphEditor->ClearSelectionSet();

		// Grab the text to paste from the clipboard.
		FString TextToImport;
		FPlatformApplicationMisc::ClipboardPaste(TextToImport);

		// Import the nodes
		TSet<UEdGraphNode*> PastedNodes;
		FEdGraphUtilities::ImportNodesFromText(EdGraph, TextToImport, PastedNodes);

		//Average position of nodes so we can move them while still maintaining relative distances to each other
		FVector2D AvgNodePosition(0.0f, 0.0f);

		for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
		{
			UEdGraphNode* Node = *It;
			AvgNodePosition.X += Node->NodePosX;
			AvgNodePosition.Y += Node->NodePosY;

			
			if (ShouldGetNewID)
			{
				if (UDialogBuilderEdNode* EdNode = Cast<UDialogBuilderEdNode>(*It))
				{
					UQuestBuilderNode* QuestNode = EdNode ? Cast<UQuestBuilderNode>(EdNode->NodeInstance) : nullptr;
					QuestNode->ID = FDialogBuilderEditorUtils::FindUniqueNodeName(QuestNode->ID.ToString());
					if (UDialogBuilderEdGraph* QuestEdGraph = Cast<UDialogBuilderEdGraph>(EdGraph))
					{
						QuestEdGraph->AddToNodeIDMap(QuestNode->ID, QuestNode);
					}
				}
			}
			else
			{
				ShouldGetNewID = true;
			}
		}

		float InvNumNodes = 1.0f / float(PastedNodes.Num());
		AvgNodePosition.X *= InvNumNodes;
		AvgNodePosition.Y *= InvNumNodes;

		for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
		{
			UEdGraphNode* Node = *It;
			CurrentGraphEditor->SetNodeSelection(Node, true);

			Node->NodePosX = (Node->NodePosX - AvgNodePosition.X) + Location.X;
			Node->NodePosY = (Node->NodePosY - AvgNodePosition.Y) + Location.Y;

			Node->SnapToGrid(16);

			// Give new node a different Guid from the old one
			Node->CreateNewGuid();

		}

	}

	// Update UI
	CurrentGraphEditor->NotifyGraphChanged();

	UObject* GraphOwner = EdGraph->GetOuter();
	if (GraphOwner)
	{
		GraphOwner->PostEditChange();
		GraphOwner->MarkPackageDirty();
	}
}

bool FDialogBuilderEditor::CanPasteNodes() const
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (!CurrentGraphEditor.IsValid())
	{
		return false;
	}

	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);

	return FEdGraphUtilities::CanImportNodesFromText(CurrentGraphEditor->GetCurrentGraph(), ClipboardContent);
}
void FDialogBuilderEditor::DuplicateNodes()
{
	ShouldGetNewID = true;
	CopySelectedNodes();
	PasteNodes();
}
bool FDialogBuilderEditor::CanDuplicateNodes()
{
	return CanCopyNodes();
}
void FDialogBuilderEditor::CreateNewTask()
{
	HandleNewNodeClassPicked(UQuestTask::StaticClass());
}

void FDialogBuilderEditor::HandleNewNodeClassPicked(UClass* InClass) const
{
	/*UE_CLOG(InClass == nullptr, LogBehaviorTreeEditor, Error, TEXT("Trying to handle new node of NULL class for Behavior Treee %s ")
		, *GetNameSafe(BehaviorTree));*/

	if (EditingQuestGraph != nullptr && InClass != nullptr && EditingQuestGraph->GetOutermost())
	{
		const FString ClassName = FBlueprintEditorUtils::GetClassNameWithoutSuffix(InClass);

		FString PathName = EditingQuestGraph->GetOutermost()->GetPathName();
		PathName = FPaths::GetPath(PathName);

		// Now that we've generated some reasonable default locations/names for the package, allow the user to have the final say
		// before we create the package and initialize the blueprint inside of it.
		FSaveAssetDialogConfig SaveAssetDialogConfig;
		SaveAssetDialogConfig.DialogTitleOverride = LOCTEXT("SaveAssetDialogTitle", "Save Asset As");
		SaveAssetDialogConfig.DefaultPath = PathName;
		SaveAssetDialogConfig.DefaultAssetName = ClassName + TEXT("_New");
		SaveAssetDialogConfig.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::Disallow;

		const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		const FString SaveObjectPath = ContentBrowserModule.Get().CreateModalSaveAssetDialog(SaveAssetDialogConfig);
		if (!SaveObjectPath.IsEmpty())
		{
			const FString SavePackageName = FPackageName::ObjectPathToPackageName(SaveObjectPath);
			const FString SavePackagePath = FPaths::GetPath(SavePackageName);
			const FString SaveAssetName = FPaths::GetBaseFilename(SavePackageName);

			UPackage* Package = CreatePackage(*SavePackageName);
			if (ensure(Package))
			{
				// Create and init a new Blueprint
				if (UBlueprint* NewBP = FKismetEditorUtilities::CreateBlueprint(InClass, Package, FName(*SaveAssetName), BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass()))
				{
					GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(NewBP);

					// Notify the asset registry
					FAssetRegistryModule::AssetCreated(NewBP);

					// Mark the package dirty...
					Package->MarkPackageDirty();
				}
			}
		}
	}

	FSlateApplication::Get().DismissAllMenus();
}

bool FDialogBuilderEditor::CanCreateNewTask() const
{
	return true;
}
bool FDialogBuilderEditor::IsNewTaskButtonVisible() const
{
	return true;
}

void FDialogBuilderEditor::CreateNewQuestCondition()
{
	HandleNewNodeClassPicked(UOrionDecorator::StaticClass());
}
bool FDialogBuilderEditor::CanCreateQuestCondition() const
{
	return true;
}
void FDialogBuilderEditor::CreateNewQuestEvent()
{
	HandleNewNodeClassPicked(UOrionEvent::StaticClass());
}
bool FDialogBuilderEditor::CanCreateQuestEvent() const
{
	return true;
}
void FDialogBuilderEditor::OpenQuestSetting()
{
	PropertyWidget->SetObject(GetQuestBuilderGraph());
}
bool FDialogBuilderEditor::CanOpenQuestSetting() const
{
	return true;
}
void FDialogBuilderEditor::OnRenameNode()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
		for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
		{
			UEdGraphNode* SelectedNode = Cast<UEdGraphNode>(*NodeIt);
			if (SelectedNode != NULL && SelectedNode->bCanRenameNode)
			{
				CurrentGraphEditor->IsNodeTitleVisible(SelectedNode, true);
				break;
			}
		}
	}
}
bool FDialogBuilderEditor::CanRenameNodes() const
{
	if (GetFocusedGraph())
	{
		if (const UEdGraphNode* SelectedNode = GetSingleSelectedNode())
		{
			return SelectedNode->GetCanRenameNode();
		}
	}
	return false;

}
void FDialogBuilderEditor::OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection)
{
	TArray<UObject*> Selection = FDialogBuilderEditorUtils::GetSelectionForPropertyEditor(NewSelection);

	if (Selection.Num() == 0)
	{
		if (UDialogBuilderEdGraph* QuestGraph = Cast<UDialogBuilderEdGraph>(ViewportWidget->GetCurrentGraph()))
		{
			PropertyWidget->SetObject(QuestGraph->Dialog);
		}
	}
	else if (Selection.Num() == 1)
	{
		if (UQuestBuilderNode_Root* RootNode = Cast<UQuestBuilderNode_Root>(Selection[0]))
		{
			if (UDialogBuilderEdGraph* QuestGraph = Cast<UDialogBuilderEdGraph>(ViewportWidget->GetCurrentGraph()))
			{
				PropertyWidget->SetObject(QuestGraph->Dialog);
			}
		}
		else
		{
			PropertyWidget->SetObjects(Selection);
		}
	}
	else
	{
		PropertyWidget->SetObjects(Selection);
	}
}

void FDialogBuilderEditor::OnNodeDoubleClicked(UEdGraphNode* Node)
{
	UDialogBuilderEdNode* QuestEdNode = Cast<UDialogBuilderEdNode>(Node);
	UQuestBuilderNode* QuestNode = QuestEdNode ? Cast<UQuestBuilderNode>(QuestEdNode->NodeInstance) : nullptr;

	if (QuestEdNode && QuestNode &&
		QuestNode->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UClass* NodeClass = QuestNode->GetClass();
		UPackage* Pkg = NodeClass->GetOuterUPackage();
		FString ClassName = NodeClass->GetName().LeftChop(2);
		UBlueprint* BlueprintOb = FindObject<UBlueprint>(Pkg, *ClassName);
		if (BlueprintOb)
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(BlueprintOb);
		}
	}
}

void FDialogBuilderEditor::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (EditingQuestGraph == nullptr)
		return;

	for (UEdGraph* EdGraph : EditingQuestGraph->QuestGraphPages)
	{
		EdGraph->GetSchema()->ForceVisualizationCacheClear();
	}
	DocumentManager->RefreshAllTabs();

}
#include "Async/Async.h"

void FDialogBuilderEditor::OnPackageMarkedDirty(UPackage* ModifiedPackage, bool bWasDirty)
{
	if (EditingQuestGraph)
	{
		for (UEdGraph* EdGraph : EditingQuestGraph->QuestGraphPages)
		{
			if (EdGraph)
			{
				if (UDialogBuilderEdGraph* QuestGraph = Cast<UDialogBuilderEdGraph>(EdGraph))
				{
					QuestGraph->SortAllChildrenNodes();
				}
			}
		}

	}
   
    
}
//Called when saving our file graph
#if ENGINE_MAJOR_VERSION < 5

void FDialogBuilderEditor::OnPackageSaved(const FString& PackageFileName, UObject* Outer)
{
	RebuildQuestBuilderGraphPages();
}
#else // #if ENGINE_MAJOR_VERSION < 5
void FDialogBuilderEditor::OnPackageSavedWithContext(const FString& PackageFileName, UPackage* Package, FObjectPostSaveContext ObjectSaveContext)
{
	RebuildQuestBuilderGraphPages();
	
}

#endif // #else // #if ENGINE_MAJOR_VERSION < 5

#undef LOCTEXT_NAMESPACE
