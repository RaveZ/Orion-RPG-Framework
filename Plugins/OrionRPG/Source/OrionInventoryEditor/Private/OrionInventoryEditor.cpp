// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionInventoryEditor.h"
#include "OrionInventoryEditorModule.h"
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
#include "OrionInventoryEditorToolbar.h"
#include "OrionInventory_EditorCommands.h"
#include "OrionInventoryEdGraph.h"
#include "OrionInventoryEdNode.h"
#include "OrionInventoryEdSubNode_Item.h"
#include "Inventory/OrionInventoryNode_Item.h"
#include "Inventory/OrionInventoryNode.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "K2Node.h"
#include "GraphEditAction.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "HAL/PlatformApplicationMisc.h"
#include "EdGraphSchema_OrionInventory.h"
#include "OrionInventoryEditorUtils.h"
#include "OrionInventoryFactory.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "SMyInventory.h"
#include "ContentBrowserModule.h"
#include "ContentBrowserFrontEndFilterExtension.h"
#include "Kismet2/KismetDebugUtilities.h"
#include "WorkflowOrientedApp/WorkflowUObjectDocuments.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include <Kismet2/KismetEditorUtilities.h>

#define LOCTEXT_NAMESPACE "OrionInventoryEditor"

const FName FOrionInventoryEditorTabs::MyInventoryDetailID(TEXT("MyInventoryDetail"));
const FName FOrionInventoryEditorTabs::InventoryPropertyID(TEXT("OrionInventoryProperty"));
const FName FOrionInventoryEditorTabs::ViewportID(TEXT("Viewport"));
const FName FOrionInventoryEditorTabs::InventoryEditorSettingsID(TEXT("OrionInventoryEditorSettings"));

const FName OrionInventoryEditorAppName = FName(TEXT("OrionInventoryEditorApp"));

//////////////////////////////////////////////////////////////////////////
FOrionInventoryEditor::FOrionInventoryEditor()
{
	EditingInventoryGraph = nullptr;

#if ENGINE_MAJOR_VERSION < 5
	OnPackageSavedDelegateHandle = UPackage::PackageSavedEvent.AddRaw(this, &FOrionInventoryEditor::OnPackageSaved);
#else // #if ENGINE_MAJOR_VERSION < 5
	OnPackageSavedDelegateHandle = UPackage::PackageSavedWithContextEvent.AddRaw(this, &FOrionInventoryEditor::OnPackageSavedWithContext);
#endif // #else // #if ENGINE_MAJOR_VERSION < 5
}

FOrionInventoryEditor::~FOrionInventoryEditor()
{
#if ENGINE_MAJOR_VERSION < 5
	UPackage::PackageSavedEvent.Remove(OnPackageSavedDelegateHandle);
#else // #if ENGINE_MAJOR_VERSION < 5
	UPackage::PackageSavedWithContextEvent.Remove(OnPackageSavedDelegateHandle);
#endif // #else // #if ENGINE_MAJOR_VERSION < 5
}

void FOrionInventoryEditor::Initialize(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UObject* InObject)
{
	UOrionInventoryGraph* InventoryGraphToEdit = Cast<UOrionInventoryGraph>(InObject);

	if (InventoryGraphToEdit != nullptr)
	{
		EditingInventoryGraph = InventoryGraphToEdit;
	}

	//Binding Functionality
	EditingInventoryGraph->GetOutermost()->PackageMarkedDirtyEvent.AddRaw(this, &FOrionInventoryEditor::OnPackageMarkedDirty);

	TSharedPtr<FOrionInventoryEditor> ThisPtr(SharedThis(this));
	if (!DocumentManager.IsValid())
	{
		DocumentManager = MakeShareable(new FDocumentTracker);
		DocumentManager->Initialize(ThisPtr);

		// Register the document factories
		{
			TSharedRef<FDocumentTabFactory> GraphEditorFactory = MakeShareable(new FInventoryGraphEditorSummoner(ThisPtr,
				FInventoryGraphEditorSummoner::FOnCreateGraphEditorWidget::CreateSP(this, &FOrionInventoryEditor::CreateGraphEditorWidget)
			));

			// Also store off a reference to the grapheditor factory so we can find all the tabs spawned by it later.
			InventoryEditorTabFactoryPtr = GraphEditorFactory;
			DocumentManager->RegisterDocumentFactory(GraphEditorFactory);
		}
	}

	TArray<UObject*> ObjectsToEdit;
	if (EditingInventoryGraph != nullptr)
	{
		ObjectsToEdit.Add(EditingInventoryGraph);
	}

	//Make Toolbar
	if (!ToolbarBuilder.IsValid())
	{
		ToolbarBuilder = MakeShareable(new FOrionInventoryEditorToolbar(SharedThis(this)));
	}

	// if we are already editing objects, dont try to recreate the editor from scratch but update the list of objects in edition
	// ex: BehaviorTree may want to reuse an editor already opened for its associated Blackboard asset.
	const TArray<UObject*>* EditedObjects = GetObjectsCurrentlyBeingEdited();
	if (EditedObjects == nullptr || EditedObjects->Num() == 0)
	{
		FGenericCommands::Register();
		FGraphEditorCommands::Register();
		FMyInventoryCommands::Register();
		FOrionInventory_EditorCommands::Register();

		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

		ToolbarBuilder->AddInventorySystemToolbar(ToolbarExtender);

		BindCommands();
		CreateInternalWidgets();

		// Layout
		const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_OrionInventoryEditor_Layout_v1")
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
						->AddTab(FOrionInventoryEditorTabs::MyInventoryDetailID, ETabState::OpenedTab)
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
							->AddTab(FOrionInventoryEditorTabs::InventoryPropertyID, ETabState::OpenedTab)
						)
					)
				)
			);

		const bool bCreateDefaultStandaloneMenu = true;
		const bool bCreateDefaultToolbar = true;
		InitAssetEditor(Mode, InitToolkitHost, OrionInventoryEditorAppName, StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, EditingInventoryGraph, false);


		if (EditingInventoryGraph && EditingInventoryGraph->bIsNewlyCreated)
		{
			NewDocument_OnClicked(CGT_NewInventoryGraph);
			EditingInventoryGraph->bIsNewlyCreated = false;
			EditingInventoryGraph->Modify();
		}
		else
		{
			if (GetOrionInventoryGraph()->InventoryGraphPages.Num() > 0)
			{
				OpenDocument(GetOrionInventoryGraph()->InventoryGraphPages[0], FDocumentTracker::OpenNewDocument);
				
				//bind on graph changes
				for (UEdGraph* EdGraph : EditingInventoryGraph->InventoryGraphPages)
				{
					if (EdGraph)
					{
						if (UOrionInventoryEdGraph* InventoryGraph = Cast<UOrionInventoryEdGraph>(EdGraph))
						{
							InventoryGraph->AddOnGraphChangedHandler(FOnGraphChanged::FDelegate::CreateSP(this, &FOrionInventoryEditor::OnGraphChanged));
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
	RebuildOrionInventoryGraphPages();
}


void FOrionInventoryEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	DocumentManager->SetTabManager(InTabManager);

	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_OrionInventoryEditor", "Generic Graph Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FOrionInventoryEditorTabs::MyInventoryDetailID, FOnSpawnTab::CreateSP(this, &FOrionInventoryEditor::SpawnTab_MyInventory))
		.SetDisplayName(LOCTEXT("Inventory Details", "My Inventory"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FOrionInventoryEditorTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FOrionInventoryEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("GraphCanvasTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.EventGraph_16x"));

	InTabManager->RegisterTabSpawner(FOrionInventoryEditorTabs::InventoryPropertyID, FOnSpawnTab::CreateSP(this, &FOrionInventoryEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab", "Property"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FOrionInventoryEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FOrionInventoryEditorTabs::MyInventoryDetailID);
	InTabManager->UnregisterTabSpawner(FOrionInventoryEditorTabs::ViewportID);
	InTabManager->UnregisterTabSpawner(FOrionInventoryEditorTabs::InventoryPropertyID);
	InTabManager->UnregisterTabSpawner(FOrionInventoryEditorTabs::InventoryEditorSettingsID);
}

void FOrionInventoryEditor::PostUndo(bool bSuccess)
{
	if (bSuccess)
	{
		// Clear selection, to avoid holding refs to nodes that go away
		if (TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor())
		{
			CurrentGraphEditor->ClearSelectionSet();
			CurrentGraphEditor->NotifyGraphChanged();
		}
		FSlateApplication::Get().DismissAllMenus();
	}
}

void FOrionInventoryEditor::PostRedo(bool bSuccess)
{
	if (bSuccess)
	{
		// Clear selection, to avoid holding refs to nodes that go away
		if (TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor())
		{
			CurrentGraphEditor->ClearSelectionSet();
			CurrentGraphEditor->NotifyGraphChanged();
		}
		FSlateApplication::Get().DismissAllMenus();
	}
}

void FOrionInventoryEditor::BindCommands()
{
	ToolkitCommands->MapAction(FOrionInventory_EditorCommands::Get().AddInventoryItem,
		FExecuteAction::CreateSP(this, &FOrionInventoryEditor::CreateNewInventoryItem),
		FCanExecuteAction::CreateSP(this, &FOrionInventoryEditor::CanCreateInventoryItem)
	);

	ToolkitCommands->MapAction(FOrionInventory_EditorCommands::Get().InventorySetting,
		FExecuteAction::CreateSP(this, &FOrionInventoryEditor::OpenInventorySetting),
		FCanExecuteAction::CreateSP(this, &FOrionInventoryEditor::CanOpenInventorySetting)
	);
	
	ToolkitCommands->MapAction(FOrionInventory_EditorCommands::Get().AddNewInventoryGraph,
		FExecuteAction::CreateSP(this, &FOrionInventoryEditor::NewDocument_OnClicked, CGT_NewInventoryGraph),
		FCanExecuteAction::CreateSP(this, &FOrionInventoryEditor::CanAddNewInventoryGraph),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateSP(this, &FOrionInventoryEditor::NewDocument_IsVisibleForType, CGT_NewInventoryGraph)
	);

}

void FOrionInventoryEditor::CreateCommandList()
{
	if (GraphEditorCommands.IsValid()) {
		return;
	}

	GraphEditorCommands = MakeShareable(new FUICommandList);
	// Can't use CreateSP here because derived editor are already implementing TSharedFromThis<FAssetEditorToolkit>
	// however it should be safe, since commands are being used only within this editor
	// if it ever crashes, this function will have to go away and be reimplemented in each derived class

	GraphEditorCommands->MapAction(FOrionInventory_EditorCommands::Get().AddNewInventoryGraph,
		FExecuteAction::CreateSP(this, &FOrionInventoryEditor::NewDocument_OnClicked, CGT_NewInventoryGraph),
		FCanExecuteAction::CreateSP(this, &FOrionInventoryEditor::CanAddNewInventoryGraph),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateSP(this, &FOrionInventoryEditor::NewDocument_IsVisibleForType, CGT_NewInventoryGraph)
	);

	GraphEditorCommands->MapAction(FOrionInventory_EditorCommands::Get().AddInventoryItem,
		FExecuteAction::CreateRaw(this, &FOrionInventoryEditor::CreateNewInventoryItem),
		FCanExecuteAction::CreateRaw(this, &FOrionInventoryEditor::CanCreateInventoryItem));
	
	GraphEditorCommands->MapAction(FOrionInventory_EditorCommands::Get().InventorySetting,
		FExecuteAction::CreateRaw(this, &FOrionInventoryEditor::OpenInventorySetting),
		FCanExecuteAction::CreateRaw(this, &FOrionInventoryEditor::CanOpenInventorySetting));


	GraphEditorCommands->MapAction(FGenericCommands::Get().SelectAll,
		FExecuteAction::CreateRaw(this, &FOrionInventoryEditor::SelectAllNodes),
		FCanExecuteAction::CreateRaw(this, &FOrionInventoryEditor::CanSelectAllNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Delete,
		FExecuteAction::CreateRaw(this, &FOrionInventoryEditor::DeleteSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FOrionInventoryEditor::CanDeleteNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Copy,
		FExecuteAction::CreateRaw(this, &FOrionInventoryEditor::CopySelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FOrionInventoryEditor::CanCopyNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Cut,
		FExecuteAction::CreateRaw(this, &FOrionInventoryEditor::CutSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FOrionInventoryEditor::CanCutNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Paste,
		FExecuteAction::CreateRaw(this, &FOrionInventoryEditor::PasteNodes),
		FCanExecuteAction::CreateRaw(this, &FOrionInventoryEditor::CanPasteNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Duplicate,
		FExecuteAction::CreateRaw(this, &FOrionInventoryEditor::DuplicateNodes),
		FCanExecuteAction::CreateRaw(this, &FOrionInventoryEditor::CanDuplicateNodes)
	);

	GraphEditorCommands->MapAction(FGenericCommands::Get().Rename,
		FExecuteAction::CreateSP(this, &FOrionInventoryEditor::OnRenameNode),
		FCanExecuteAction::CreateSP(this, &FOrionInventoryEditor::CanRenameNodes)
	);

	GraphEditorCommands->MapAction(
		FGraphEditorCommands::Get().CreateComment,
		FExecuteAction::CreateRaw(this, &FOrionInventoryEditor::OnCreateComment),
		FCanExecuteAction::CreateRaw(this, &FOrionInventoryEditor::CanCreateComment)
	);
}

TSharedPtr<SGraphEditor> FOrionInventoryEditor::GetCurrGraphEditor() const
{
	return CurrentGraphWidget;
}

FGraphPanelSelectionSet FOrionInventoryEditor::GetSelectedNodes() const
{
	FGraphPanelSelectionSet CurrentSelection;
	TSharedPtr<SGraphEditor> FocusedGraphEd = GetCurrGraphEditor();
	if (FocusedGraphEd.IsValid())
	{
		CurrentSelection = FocusedGraphEd->GetSelectedNodes();
	}

	return CurrentSelection;
}

FName FOrionInventoryEditor::GetToolkitFName() const
{
	return FName("FInventoryGraphEditor");
}

FText FOrionInventoryEditor::GetBaseToolkitName() const
{
	return LOCTEXT("InventoryGraphEditorAppLabel", "Inventory Graph Editor");
}


FText FOrionInventoryEditor::GetToolkitName() const
{
	const bool bDirtyState = EditingInventoryGraph->GetOutermost()->IsDirty();

	FFormatNamedArguments Args;
	Args.Add(TEXT("InventoryGraphName"), FText::FromString(EditingInventoryGraph->GetName()));
	Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
	return FText::Format(LOCTEXT("InventoryGraphEditorToolkitName", "{InventoryGraphName}{DirtyState}"), Args);
}

FText FOrionInventoryEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(EditingInventoryGraph);
}

FLinearColor FOrionInventoryEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::Blue;
}

FString FOrionInventoryEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("InventoryGraphEditor");
}

FString FOrionInventoryEditor::GetDocumentationLink() const
{
	//make documentation from notion add this link
	return TEXT("");
}

void FOrionInventoryEditor::SaveAsset_Execute()
{
	FAssetEditorToolkit::SaveAsset_Execute();

}

void FOrionInventoryEditor::RefreshEditors()
{
	
}

void FOrionInventoryEditor::RefreshMyInventory()
{
}

void FOrionInventoryEditor::RefreshInspector()
{
}

void FOrionInventoryEditor::AddToSelection(UEdGraphNode* InNode)
{
}

void FOrionInventoryEditor::JumpToHyperlink(const UObject* ObjectReference, bool bReinventoryRename)
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

void FOrionInventoryEditor::JumpToPin(const UEdGraphPin* Pin)
{
}

void FOrionInventoryEditor::SummonSearchUI(bool bSetFindWithinBlueprint, FString NewSearchTerms, bool bSelectFirstResult)
{
}

void FOrionInventoryEditor::SummonFindAndReplaceUI()
{
}

TSharedPtr<SGraphEditor> FOrionInventoryEditor::OpenGraphAndBringToFront(UEdGraph* Graph, bool bSetFocus)
{
	return TSharedPtr<SGraphEditor>();
}

void FOrionInventoryEditor::UpdateToolbar()
{
}

void FOrionInventoryEditor::RegisterToolbarTab(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
}

void FOrionInventoryEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	/*if (GetObjectsCurrentlyBeingEdited()->Num() > 0)
	{
		TArray<UObject*>& LocalEditingObjects = const_cast<TArray<UObject*>&>(GetEditingObjects());

		Collector.AddReferencedObjects(LocalEditingObjects);
	}

	Collector.AddReferencedObject(EditingInventoryGraph);*/
}

FString FOrionInventoryEditor::GetReferencerName() const
{
	return FString();
}

UOrionInventoryGraph* FOrionInventoryEditor::GetOrionInventoryGraph() const
{
	return EditingInventoryGraph;
}

bool FOrionInventoryEditor::NewDocument_IsVisibleForType(ECreatedInventoryDocumentType GraphType) const
{
	return false;
}

void FOrionInventoryEditor::NewDocument_OnClicked(ECreatedInventoryDocumentType GraphType)
{
	FText DocumentNameText;
	bool bResetMyBlueprintFilter = false;

	switch (GraphType)
	{
	case CGT_NewInventoryGraph:
		DocumentNameText = LOCTEXT("NewDocInventoryName", "Inventory Graph");
		bResetMyBlueprintFilter = true;
		break;
	
	default:
		DocumentNameText = LOCTEXT("NewDocNewName", "NewDocument");
		break;
	}

	FName DocumentName = FName(*DocumentNameText.ToString());

	

	// Make sure the new name is valid
	DocumentName = FOrionInventoryEditorUtils::FindUniqueInventoryName(DocumentNameText.ToString());
		
	//check(IsEditingSingleBlueprint());

	const FScopedTransaction Transaction(LOCTEXT("AddNewInventoryGraph", "Add New Inventory Graph"));
	GetOrionInventoryGraph()->Modify();

	UEdGraph* NewGraph = nullptr;

	
	if (GraphType == CGT_NewInventoryGraph)
	{
		NewGraph = FOrionInventoryEditorUtils::CreateNewGraph(GetOrionInventoryGraph(), DocumentName, UOrionInventoryEdGraph::StaticClass(), UEdGraphSchema_OrionInventory::StaticClass());
		NewGraph->AddOnGraphChangedHandler(FOnGraphChanged::FDelegate::CreateSP(this, &FOrionInventoryEditor::OnGraphChanged));
		FOrionInventoryEditorUtils::AddInventoryGraphPage(GetOrionInventoryGraph(), NewGraph);

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

bool FOrionInventoryEditor::InEditingMode() const
{
	return true;
}

bool FOrionInventoryEditor::CanAddNewInventoryGraph() const
{
	return false;
}

TSharedPtr<SDockTab> FOrionInventoryEditor::OpenDocument(const UObject* DocumentID, FDocumentTracker::EOpenDocumentCause Cause)
{
	TSharedRef<FTabPayload_UObject> Payload = FTabPayload_UObject::Make(DocumentID);
	return DocumentManager->OpenDocument(Payload, Cause);
}

void FOrionInventoryEditor::CloseDocumentTab(const UObject* DocumentID)
{
	TSharedRef<FTabPayload_UObject> Payload = FTabPayload_UObject::Make(DocumentID);
	DocumentManager->CloseTab(Payload);
}

void FOrionInventoryEditor::RenameNewlyAddedAction(FName InActionName)
{
	if (MyInventoryWidget.IsValid())
	{
		// Force a refresh immediately, the item has to be present in the list for the rename reinventorys to be successful.
		MyInventoryWidget->Refresh();
		MyInventoryWidget->SelectItemByName(InActionName, ESelectInfo::OnMouseClick);
		MyInventoryWidget->OnRequestRenameOnActionNode();
	}
}

void FOrionInventoryEditor::LogSimpleMessage(const FText& MessageText)
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

TSharedRef<SWidget> FOrionInventoryEditor::CreateGraphTitleBarWidget(TSharedRef<FTabInfo> InTabInfo, UEdGraph* InGraph)
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


const FSlateBrush* FOrionInventoryEditor::GetGlyphForGraph(const UEdGraph* Graph, bool bInLargeIcon)
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

bool FOrionInventoryEditor::FindOpenTabsContainingDocument(const UObject* DocumentID, TArray<TSharedPtr<SDockTab>>& Results)
{
	int32 StartingCount = Results.Num();

	TSharedRef<FTabPayload_UObject> Payload = FTabPayload_UObject::Make(DocumentID);

	DocumentManager->FindMatchingTabs(Payload, /*inout*/ Results);

	// Did we add anything new?
	return (StartingCount != Results.Num());
}


void FOrionInventoryEditor::InitializeDocumentTab()
{
	check(IsEditingSingleInventoryGraph());

	UOrionInventoryGraph* OrionInventoryGraph = GetOrionInventoryGraph();
	if (OrionInventoryGraph->LastEditedDocuments.Num() == 0)
	{
			OrionInventoryGraph->LastEditedDocuments.Add(FOrionInventoryEditorUtils::FindInventoryGraph(OrionInventoryGraph));
	}

	for (int32 i = 0; i < OrionInventoryGraph->LastEditedDocuments.Num(); i++)
	{
		if (UObject* Obj = OrionInventoryGraph->LastEditedDocuments[i].EditedObjectPath.ResolveObject())
		{
			if (UEdGraph* Graph = Cast<UEdGraph>(Obj))
			{
				struct LocalStruct
				{
					static TSharedPtr<SDockTab> OpenGraphTree(FOrionInventoryEditor* InInventorySystemGraphEditor, UEdGraph* InGraph)
					{
						FDocumentTracker::EOpenDocumentCause OpenCause = FDocumentTracker::QuickNavigateCurrentDocument;

						for (UObject* OuterObject = InGraph->GetOuter(); OuterObject; OuterObject = OuterObject->GetOuter())
						{
							if (OuterObject->IsA<UOrionInventoryGraph>())
							{
								// reached up to the OrionInventoryGraph for the graph, we are done climbing the tree
								OpenCause = FDocumentTracker::RestorePreviousDocument;
								break;
							}
							else if (UEdGraph* OuterGraph = Cast<UEdGraph>(OuterObject))
							{
								// Found another graph, open it up
								OpenGraphTree(InInventorySystemGraphEditor, OuterGraph);
								break;
							}
						}

						return InInventorySystemGraphEditor->OpenDocument(InGraph, OpenCause);
					}
				};
				TSharedPtr<SDockTab> TabWithGraph = LocalStruct::OpenGraphTree(this, Graph);
				if (TabWithGraph.IsValid())
				{
					TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(TabWithGraph->GetContent());
					GraphEditor->SetViewLocation(OrionInventoryGraph->LastEditedDocuments[i].SavedViewOffset, OrionInventoryGraph->LastEditedDocuments[i].SavedZoomAmount);
				}
			}
			else
			{
				TSharedPtr<SDockTab> TabWithGraph = OpenDocument(Obj, FDocumentTracker::RestorePreviousDocument);
			}
		}
	}
}



bool FOrionInventoryEditor::IsEditingSingleInventoryGraph() const
{
	return GetOrionInventoryGraph() != nullptr;
}

UEdGraph* FOrionInventoryEditor::GetFocusedGraph() const
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

UEdGraphNode* FOrionInventoryEditor::GetSingleSelectedNode() const
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	return (SelectedNodes.Num() == 1) ? Cast<UEdGraphNode>(*SelectedNodes.CreateConstIterator()) : nullptr;
}

void FOrionInventoryEditor::OnGraphEditorFocused(const TSharedRef<class SGraphEditor>& InGraphEditor)
{
	// Update the graph editor that is currently focused
	CurrentGraphWidget = InGraphEditor;
	InGraphEditor->SetPinVisibility(SGraphEditor::EPinVisibility::Pin_Show);

	// Update the inspector as well, to show selection from the focused graph editor
	FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	//FocusInspectorOnGraphSelection(SelectedNodes, /*bForceRefresh=*/ true);

	// During undo, garbage graphs can be temporarily brought into focus, ensure that before a refresh of the MyBlueprint window that the graph is owned by a Blueprint
	if (CurrentGraphWidget.IsValid() && MyInventoryWidget.IsValid())
	{
		// The focused graph can be garbage as well
		TWeakObjectPtr< UEdGraph > FocusedGraphPtr = CurrentGraphWidget->GetCurrentGraph();
		UEdGraph* FocusedGraph = FocusedGraphPtr.Get();
		
		if (FocusedGraph != nullptr)
		{
			if (UOrionInventoryEdGraph* InventoryEdGraph = Cast<UOrionInventoryEdGraph>(FocusedGraph))
			{
				InventoryEdGraph->SEditorGraph = CurrentGraphWidget.Get();
			}
			MyInventoryWidget->Refresh();
		}
	}

	
}

void FOrionInventoryEditor::OnGraphEditorBackgrounded(const TSharedRef<SGraphEditor>& InGraphEditor)
{
}

bool FOrionInventoryEditor::IsGraphInCurrentInventoryGraph(const UEdGraph* InGraph) const
{
	bool bEditable = true;

	UOrionInventoryGraph* EditingBP = GetOrionInventoryGraph();
	if (EditingBP)
	{
		TArray<UEdGraph*> Graphs;
		EditingBP->GetAllGraphs(Graphs);
		bEditable &= Graphs.Contains(InGraph);
	}

	return bEditable;
}

FGraphAppearanceInfo FOrionInventoryEditor::GetGraphAppearance() const
{
	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText", "Inventory Editor");

	if (FOrionInventoryEditor::IsPIESimulating())
	{
		if (GetOrionInventoryGraph()->OrionInventoryComponent)
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

bool FOrionInventoryEditor::InEditingMode(bool bGraphIsEditable) const
{
	return bGraphIsEditable && FOrionInventoryEditor::IsPIENotSimulating();
}


bool FOrionInventoryEditor::IsPIESimulating()
{
	return GEditor->bIsSimulatingInEditor || GEditor->PlayWorld;
}

bool FOrionInventoryEditor::IsPIENotSimulating()
{
	return !GEditor->bIsSimulatingInEditor && (GEditor->PlayWorld == NULL);
}

void FOrionInventoryEditor::OnChangeBreadCrumbGraph(UEdGraph* InGraph)
{
}

TSharedRef<SDockTab> FOrionInventoryEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FOrionInventoryEditorTabs::ViewportID);

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.Label(LOCTEXT("ViewportTab_Title", "Viewport"));

	if (CurrentGraphWidget.IsValid())
	{
		SpawnedTab->SetContent(CurrentGraphWidget.ToSharedRef());
	}

	return SpawnedTab;
}

TSharedRef<SDockTab> FOrionInventoryEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FOrionInventoryEditorTabs::InventoryPropertyID);

	return SNew(SDockTab)
#if ENGINE_MAJOR_VERSION < 5
		.Icon(FAppStyle::GetBrush("LevelEditor.Tabs.Details"))
#endif // #if ENGINE_MAJOR_VERSION < 5
		.Label(LOCTEXT("Details_Title", "Property"))
		[
			PropertyWidget.ToSharedRef()
		];
}

TSharedRef<SDockTab> FOrionInventoryEditor::SpawnTab_EditorSettings(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FOrionInventoryEditorTabs::InventoryEditorSettingsID);

	return SNew(SDockTab)
#if ENGINE_MAJOR_VERSION < 5
		.Icon(FAppStyle::GetBrush("LevelEditor.Tabs.Details"))
#endif // #if ENGINE_MAJOR_VERSION < 5
		.Label(LOCTEXT("EditorSettings_Title", "Generic Graph Editor Setttings"))
		[
			EditorSettingsWidget.ToSharedRef()
		];
}

TSharedRef<SDockTab> FOrionInventoryEditor::SpawnTab_MyInventory(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FOrionInventoryEditorTabs::MyInventoryDetailID);

	return SNew(SDockTab)
#if ENGINE_MAJOR_VERSION < 5
		.Icon(FAppStyle::GetBrush("LevelEditor.Tabs.Details"))
#endif // #if ENGINE_MAJOR_VERSION < 5
		.Label(LOCTEXT("MyInventory_Title", "My Inventory"))
		[
			MyInventoryWidget.ToSharedRef()
		];
}

void FOrionInventoryEditor::CreateInternalWidgets()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.NotifyHook = this;
	DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;	

	PropertyWidget = PropertyModule.CreateDetailView(DetailsViewArgs);
	PropertyWidget->SetObject( NULL );
	PropertyWidget->OnFinishedChangingProperties().AddSP(this, &FOrionInventoryEditor::OnFinishedChangingProperties);


	this->MyInventoryWidget = SNew(SMyInventory, SharedThis(this));
}

TSharedRef<SGraphEditor> FOrionInventoryEditor::CreateGraphEditorWidget(TSharedRef<class FTabInfo> InTabInfo, UEdGraph* InGraph)
{

	// Create the title bar widget
	TSharedPtr<SWidget> TitleBarWidget = CreateGraphTitleBarWidget(InTabInfo, InGraph);

	CreateCommandList();

	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FOrionInventoryEditor::OnSelectedNodesChanged);
	InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FOrionInventoryEditor::OnNodeDoubleClicked);

	// Make full graph editor
	const bool bGraphIsEditable = InGraph->bEditable;
	return SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.IsEditable(this, &FOrionInventoryEditor::InEditingMode, bGraphIsEditable)
		.TitleBar(TitleBarWidget)
		.Appearance(this, &FOrionInventoryEditor::GetGraphAppearance)
		.GraphToEdit(InGraph)
		.GraphEvents(InEvents)
		.AutoExpandActionMenu(true);
}


void FOrionInventoryEditor::RebuildOrionInventoryGraphPages()
{
	if (EditingInventoryGraph == nullptr)
	{
		return;
	}

	for (UEdGraph* EdGraph : EditingInventoryGraph->InventoryGraphPages)
	{
		if (EdGraph)
		{
			if (UOrionInventoryEdGraph* InventoryGraph = Cast<UOrionInventoryEdGraph>(EdGraph))
			{
				InventoryGraph->UpdateAsset();
			}
		}
	}
}

	



void FOrionInventoryEditor::OnGraphChanged(const FEdGraphEditAction& Action)
{
}
void FOrionInventoryEditor::SelectAllNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		CurrentGraphEditor->SelectAllNodes();
	}
}
bool FOrionInventoryEditor::CanSelectAllNodes()
{
	return true;
}
void FOrionInventoryEditor::DeleteSelectedNodes()
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

		if (UOrionInventoryEdNode* InventoryEdNode = Cast<UOrionInventoryEdNode>(EdNode))
		{
			InventoryEdNode->Modify();

			const UEdGraphSchema* Schema = InventoryEdNode->GetSchema();
			if (Schema != nullptr)
			{
				Schema->BreakNodeLinks(*InventoryEdNode);
			}

			InventoryEdNode->DestroyNode();
		}
		else
		{
			EdNode->Modify();
			EdNode->DestroyNode();
		}

	}
}
bool FOrionInventoryEditor::CanDeleteNodes()
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
void FOrionInventoryEditor::DeleteSelectedDuplicatableNodes()
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
void FOrionInventoryEditor::CutSelectedNodes()
{
	CopySelectedNodes();
	DeleteSelectedDuplicatableNodes();
	ShouldGetNewID = false;
}
bool FOrionInventoryEditor::CanCutNodes()
{
	if (IsPIESimulating())
		return false;
	return CanCopyNodes() && CanDeleteNodes();
}

void FOrionInventoryEditor::CopySelectedNodes()
{
	// Export the selected nodes and place the text on the clipboard
	FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	TArray<UOrionInventoryEdNode*> SubNodes;

	FString ExportedText;

	ShouldGetNewID = true;

	int32 CopySubNodeIndex = 0;
	for (FGraphPanelSelectionSet::TIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		UOrionInventoryEdNode* InventoryEdNode = Cast<UOrionInventoryEdNode>(Node);
		if (Node == nullptr)
		{
			SelectedIter.RemoveCurrent();
			continue;
		}


		Node->PrepareForCopying();

		if (InventoryEdNode)
		{
			InventoryEdNode->CopySubNodeIndex = CopySubNodeIndex;

			// append all subnodes for selection
			for (int32 Idx = 0; Idx < InventoryEdNode->SubNodes.Num(); Idx++)
			{
				InventoryEdNode->SubNodes[Idx]->CopySubNodeIndex = CopySubNodeIndex;
				SubNodes.Add(InventoryEdNode->SubNodes[Idx]);
			}

			CopySubNodeIndex++;
		}

	}

	for (int32 Idx = 0; Idx < SubNodes.Num(); Idx++)
	{
		SelectedNodes.Add(SubNodes[Idx]);
		SubNodes[Idx]->PrepareForCopying();
	}

	FEdGraphUtilities::ExportNodesToText(SelectedNodes, ExportedText);
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);

	for (FGraphPanelSelectionSet::TIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UOrionInventoryEdNode* Node = Cast<UOrionInventoryEdNode>(*SelectedIter);
		if (Node)
		{
			Node->PostCopyNode();
		}
	}
}

bool FOrionInventoryEditor::CanCopyNodes()
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
void FOrionInventoryEditor::PasteNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		PasteNodesHere(CurrentGraphEditor->GetCurrentGraph(), CurrentGraphEditor->GetPasteLocation());
	}
}
void FOrionInventoryEditor::PasteNodesHere(UEdGraph* DestinationGraph, const FVector2D& Location)
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
		// Undo/Redo support
		const FScopedTransaction Transaction(FGenericCommands::Get().Paste->GetDescription());
		UOrionInventoryEdGraph* InventoryEdGraph = Cast<UOrionInventoryEdGraph>(EdGraph);

		EdGraph->Modify();

		if (InventoryEdGraph)
		{
			InventoryEdGraph->LockUpdates();
		}

		UOrionInventoryEdNode* SelectedParent = NULL;
		bool bHasMultipleNodesSelected = false;

		const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
		for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
		{
			UOrionInventoryEdNode* Node = Cast<UOrionInventoryEdNode>(*SelectedIter);
			if (Node && Node->IsSubNode())
			{
				Node = Node->ParentNode;
			}

			if (Node)
			{
				if (SelectedParent == nullptr)
				{
					SelectedParent = Node;
				}
				else
				{
					bHasMultipleNodesSelected = true;
					break;
				}
			}
		}

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

		// Number of nodes used to calculate AvgNodePosition
		int32 AvgCount = 0;

		for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
		{
			UEdGraphNode* EdNode = *It;
			UOrionInventoryEdNode* InventoryEdNode = Cast<UOrionInventoryEdNode>(EdNode);
			if (EdNode && (InventoryEdNode == nullptr || !InventoryEdNode->IsSubNode()))
			{
				AvgNodePosition.X += EdNode->NodePosX;
				AvgNodePosition.Y += EdNode->NodePosY;
				++AvgCount;
			}
			
			if (ShouldGetNewID && InventoryEdNode && !InventoryEdNode->IsSubNode())
			{
				UOrionInventoryNode* InventoryNode = InventoryEdNode ? Cast<UOrionInventoryNode>(InventoryEdNode->NodeInstance) : nullptr;
				if (InventoryNode)
				{
					InventoryEdNode->FindUniqueNodeName(InventoryNode->ID.ToString());
				}
			}
			else
			{
				ShouldGetNewID = true;
			}
		}

		if (AvgCount > 0)
		{
			float InvNumNodes = 1.0f / float(AvgCount);
			AvgNodePosition.X *= InvNumNodes;
			AvgNodePosition.Y *= InvNumNodes;
		}
		
		bool bPastedParentNode = false;

		TMap<FGuid/*New*/, FGuid/*Old*/> NewToOldNodeMapping;

		TMap<int32, UOrionInventoryEdNode*> ParentMap;
		for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
		{
			UEdGraphNode* PasteNode = *It;
			UOrionInventoryEdNode* PasteInventoryEdNode = Cast<UOrionInventoryEdNode>(PasteNode);

			if (PasteNode && (PasteInventoryEdNode == nullptr || !PasteInventoryEdNode->IsSubNode()))
			{
				bPastedParentNode = true;

				// Select the newly pasted stuff
				CurrentGraphEditor->SetNodeSelection(PasteNode, true);

				const FVector::FReal NodePosX = (PasteNode->NodePosX - AvgNodePosition.X) + Location.X;
				const FVector::FReal NodePosY = (PasteNode->NodePosY - AvgNodePosition.Y) + Location.Y;

				PasteNode->NodePosX = static_cast<int32>(NodePosX);
				PasteNode->NodePosY = static_cast<int32>(NodePosY);

				PasteNode->SnapToGrid(16);

				const FGuid OldGuid = PasteNode->NodeGuid;

				// Give new node a different Guid from the old one
				PasteNode->CreateNewGuid();

				const FGuid NewGuid = PasteNode->NodeGuid;

				NewToOldNodeMapping.Add(NewGuid, OldGuid);

				if (PasteInventoryEdNode)
				{
					PasteInventoryEdNode->RemoveAllSubNodes();
					ParentMap.Add(PasteInventoryEdNode->CopySubNodeIndex, PasteInventoryEdNode);
				}
			}
		}

		for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
		{
			UOrionInventoryEdNode* PasteNode = Cast<UOrionInventoryEdNode>(*It);
			if (PasteNode && PasteNode->IsSubNode())
			{
				PasteNode->NodePosX = 0;
				PasteNode->NodePosY = 0;

				// remove subnode from graph, it will be referenced from parent node
				PasteNode->DestroyNode();

				PasteNode->ParentNode = ParentMap.FindRef(PasteNode->CopySubNodeIndex);
				if (PasteNode->ParentNode)
				{
					PasteNode->ParentNode->AddSubNode(PasteNode, EdGraph);
				}
				else if (!bHasMultipleNodesSelected && !bPastedParentNode && SelectedParent)
				{
					PasteNode->ParentNode = SelectedParent;
					SelectedParent->AddSubNode(PasteNode, EdGraph);
				}
			}
		}

		FixupPastedNodes(PastedNodes, NewToOldNodeMapping);

		if (InventoryEdGraph)
		{
			InventoryEdGraph->UpdateClassData();
			InventoryEdGraph->UnlockUpdates();
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

	
}


void FOrionInventoryEditor::FixupPastedNodes(const TSet<UEdGraphNode*>& NewPastedGraphNodes, const TMap<FGuid, FGuid>& NewToOldNodeMapping)
{
}

bool FOrionInventoryEditor::CanPasteNodes() const
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
void FOrionInventoryEditor::DuplicateNodes()
{
	ShouldGetNewID = true;
	CopySelectedNodes();
	PasteNodes();
}
bool FOrionInventoryEditor::CanDuplicateNodes()
{
	return CanCopyNodes();
}

void FOrionInventoryEditor::HandleNewNodeClassPicked(UClass* InClass) const
{

	if (EditingInventoryGraph != nullptr && InClass != nullptr && EditingInventoryGraph->GetOutermost())
	{
		const FString ClassName = FBlueprintEditorUtils::GetClassNameWithoutSuffix(InClass);

		FString PathName = EditingInventoryGraph->GetOutermost()->GetPathName();
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

void FOrionInventoryEditor::CreateNewInventoryItem()
{
    TSharedPtr<SGraphEditor> GraphEditor = GetCurrGraphEditor();
    if (!GraphEditor.IsValid())
        return;

    UEdGraph* Graph = GraphEditor->GetCurrentGraph();
    if (!Graph)
        return;


	UOrionInventoryGraph* InventoryGraph = CastChecked<UOrionInventoryGraph>(Graph->GetOuter());
	if (!InventoryGraph)
		return;

    const UEdGraphSchema* Schema = Graph->GetSchema();
    if (!Schema)
        return;

    // Assume only one node is selected
    const FGraphPanelSelectionSet SelectedNodes = GraphEditor->GetSelectedNodes();
    if (SelectedNodes.Num() != 1)
        return;

    UOrionInventoryEdNode* SelectedNode = Cast<UOrionInventoryEdNode>(*SelectedNodes.CreateConstIterator());
    if (!SelectedNode)
        return;

	if (SelectedNode->ParentNode)
		SelectedNode = SelectedNode->ParentNode;

	UClass* ItemEdNodeClass = UOrionInventoryEdSubNode_Item::StaticClass();
	UClass* ItemNodeClass = UOrionInventoryNode_Item::StaticClass();

    // Create a new subnode (replace with your subnode class if different)
    UOrionInventoryEdNode* SubNode = NewObject<UOrionInventoryEdNode>(Graph, ItemEdNodeClass);
	FGraphNodeClassData ItemClassData = FGraphNodeClassData(ItemNodeClass, "ItemClassData");
	SubNode->ClassData = ItemClassData;
	SubNode->NodeInstance = NewObject<UObject>(InventoryGraph, ItemNodeClass);
	SubNode->NodeInstance->SetFlags(RF_Transactional);

    // Add to parent node's subnodes array
	SelectedNode->AddSubNode(SubNode, Graph);

    // Notify the graph/schema of the change
    Graph->NotifyGraphChanged();
}

bool FOrionInventoryEditor::CanCreateInventoryItem() const
{
	return true;
}
void FOrionInventoryEditor::CreateNewInventoryEvent()
{
	HandleNewNodeClassPicked(UOrionEvent::StaticClass());
}
bool FOrionInventoryEditor::CanCreateInventoryEvent() const
{
	return true;
}
void FOrionInventoryEditor::OpenInventorySetting()
{
	PropertyWidget->SetObject(GetOrionInventoryGraph());
}
bool FOrionInventoryEditor::CanOpenInventorySetting() const
{
	return true;
}
void FOrionInventoryEditor::OnRenameNode()
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
bool FOrionInventoryEditor::CanRenameNodes() const
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

bool FOrionInventoryEditor::CanCreateComment() const
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	return CurrentGraphEditor.IsValid();
}

void FOrionInventoryEditor::OnCreateComment()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (UEdGraph* EdGraph = CurrentGraphEditor.IsValid() ? CurrentGraphEditor->GetCurrentGraph() : nullptr)
	{
		TSharedPtr<FEdGraphSchemaAction> Action = EdGraph->GetSchema()->GetCreateCommentAction();
		if (Action.IsValid())
		{
			Action->PerformAction(EdGraph, nullptr, FVector2D());
		}
	}
}

void FOrionInventoryEditor::OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection)
{
	TArray<UObject*> Selection = FOrionInventoryEditorUtils::GetSelectionForPropertyEditor(NewSelection);

	if (Selection.Num() == 0)
	{
		if (UOrionInventoryEdGraph* InventoryGraph = Cast<UOrionInventoryEdGraph>(CurrentGraphWidget->GetCurrentGraph()))
		{
			PropertyWidget->SetObject(InventoryGraph->OwningInventory);
		}
	}
	else if (Selection.Num() == 1)
	{
		PropertyWidget->SetObjects(Selection);
		
	}
	else
	{
		PropertyWidget->SetObjects(Selection);
	}
}

void FOrionInventoryEditor::OnNodeDoubleClicked(UEdGraphNode* Node)
{
	UOrionInventoryEdNode* InventoryEdNode = Cast<UOrionInventoryEdNode>(Node);

	if (InventoryEdNode && 
		InventoryEdNode->NodeInstance->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UClass* NodeClass = InventoryEdNode->NodeInstance->GetClass();
		UPackage* Pkg = NodeClass->GetOuterUPackage();
		FString ClassName = NodeClass->GetName().LeftChop(2);
		UBlueprint* BlueprintOb = FindObject<UBlueprint>(Pkg, *ClassName);
		if (BlueprintOb)
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(BlueprintOb);
		}
	}
}

void FOrionInventoryEditor::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (EditingInventoryGraph == nullptr)
		return;

	for (UEdGraph* EdGraph : EditingInventoryGraph->InventoryGraphPages)
	{
		EdGraph->GetSchema()->ForceVisualizationCacheClear();
	}
	DocumentManager->RefreshAllTabs();

}
#include "Async/Async.h"

void FOrionInventoryEditor::OnPackageMarkedDirty(UPackage* ModifiedPackage, bool bWasDirty)
{   
}
//Called when saving our file graph
#if ENGINE_MAJOR_VERSION < 5

void FOrionInventoryEditor::OnPackageSaved(const FString& PackageFileName, UObject* Outer)
{
	RebuildOrionInventoryGraphPages();
}
#else // #if ENGINE_MAJOR_VERSION < 5
void FOrionInventoryEditor::OnPackageSavedWithContext(const FString& PackageFileName, UPackage* Package, FObjectPostSaveContext ObjectSaveContext)
{
	RebuildOrionInventoryGraphPages();
	
}


#endif // #else // #if ENGINE_MAJOR_VERSION < 5

#undef LOCTEXT_NAMESPACE
