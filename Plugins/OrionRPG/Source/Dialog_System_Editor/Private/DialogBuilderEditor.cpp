// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "DialogBuilderEditor.h"
#include "DialogBuilderNode_Root.h"
#include "DialogBuilderEditorToolbar.h"
#include "DialogBuilder_EditorCommands.h"
#include "DialogBuilderEdGraph.h"
#include "DialogBuilderEdNode.h"
#include "DialogBuilderEditorModes.h"
#include "DialogDefinition.h"
#include "DialogSequence.h"
#include "DialogBuilderEdNode.h"
#include "DialogBuilderNode_DialogSequence.h"
#include "Decorator/OrionDecorator.h"
#include "EngineGlobals.h"
#include "Editor/EditorEngine.h"
#include "Editor.h"
#include "UnrealEdGlobals.h"
#include "Event/OrionEvent.h"
#include "DialogCameraShot.h"
#include "BlueprintEditor.h"
#include "UObject/ObjectSaveContext.h"
#include "GraphEditorActions.h"
#include "Framework/Commands/GenericCommands.h"
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
#include "SDialogDefinitions.h"
#include "Dialog_System_Editor.h"
#include "ContentBrowserModule.h"
#include "Components/BillboardComponent.h"
#include "Engine/Texture2D.h"
#include "ContentBrowserFrontEndFilterExtension.h"
#include "Kismet2/KismetDebugUtilities.h"
#include "WorkflowOrientedApp/WorkflowUObjectDocuments.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Misc/LevelSequenceEditorSpawnRegister.h"
#include "Misc/LevelSequencePlaybackContext.h"
#include "ISequencer.h"
#include "ISequencerModule.h"
#include "MovieScene.h"
#include "MovieSceneTrack.h"
#include "MovieSceneSignedObject.h"
#include "MovieSceneDialogTrack.h"
#include "Tracks/MovieSceneSkeletalAnimationTrack.h"
#include "LevelSequence.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SNullWidget.h"
#include "MovieSceneSpawnRegister.h"
#include "PreviewScene.h"
#include "SDialogPreviewViewport.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/SoftObjectPath.h"
#include "CineCameraActor.h"
#include "SequencerUtilities.h"
#include "MovieSceneToolHelpers.h"
#include "Tracks/MovieSceneCameraCutTrack.h"
#include "Bindings/MovieSceneCustomBinding.h"
#include "Bindings/MovieSceneSpawnableBinding.h"
#include "Bindings/MovieSceneReplaceableBinding.h"
#include "Bindings/MovieSceneSpawnableActorBinding.h"
#include "Bindings/MovieSceneReplaceableActorBinding.h"
#include <Kismet2/KismetEditorUtilities.h>

#define LOCTEXT_NAMESPACE "DialogBuilderEditor"

const FName FDialogBuilderEditor::DialogEditorMode(TEXT("DialogEditor"));
const FName FDialogBuilderEditor::DialogSequencerMode(TEXT("DialogSequencer"));

FText FDialogBuilderEditor::DialogEditorModeText(LOCTEXT("DialogEditorMode", "Dialog Editor"));
FText FDialogBuilderEditor::DialogSequencerModeText(LOCTEXT("DialogSequencerMode", "Dialog Sequencer"));

const FName FDialogBuilderEditorTabs::DialogDefinitionsID(TEXT("DialogDefinitions"));
const FName FDialogBuilderEditorTabs::DialogBuilderPropertyID(TEXT("DialogBuilderProperty"));
const FName FDialogBuilderEditorTabs::ViewportID(TEXT("Viewport"));
const FName FDialogBuilderEditorTabs::DialogBuilderEditorSettingsID(TEXT("DialogBuilderEditorSettings"));
const FName FDialogBuilderEditorTabs::DialogSequencerTabID(TEXT("DialogSequencerTab"));
const FName FDialogBuilderEditorTabs::DialogSequencerViewportID(TEXT("DialogSequencerViewport"));

const FName DialogBuilderEditorAppName = FName(TEXT("DialogBuilderEditorApp"));

//////////////////////////////////////////////////////////////////////////
FDialogBuilderEditor::FDialogBuilderEditor()
{
	EditingDialogGraph = nullptr;

#if ENGINE_MAJOR_VERSION < 5
	OnPackageSavedDelegateHandle = UPackage::PackageSavedEvent.AddRaw(this, &FDialogBuilderEditor::OnPackageSaved);
#else // #if ENGINE_MAJOR_VERSION < 5
	OnPackageSavedDelegateHandle = UPackage::PackageSavedWithContextEvent.AddRaw(this, &FDialogBuilderEditor::OnPackageSavedWithContext);
#endif // #else // #if ENGINE_MAJOR_VERSION < 5
}

FDialogBuilderEditor::~FDialogBuilderEditor()
{
	// Unbind raw package dirty callback added in Initialize()
	if (EditingDialogGraph && EditingDialogGraph->GetOutermost())
	{
		EditingDialogGraph->GetOutermost()->PackageMarkedDirtyEvent.RemoveAll(this);
	}

	// Release sequencer/editor references early
	Sequencer.Reset();
	PlaybackContext.Reset();
	SequencerViewportWidget.Reset();

	// Optional but recommended if you extend tool menus with this owner
	UToolMenus::UnregisterOwner(this);
	DestroyDialogDefinitions();
#if ENGINE_MAJOR_VERSION < 5
	UPackage::PackageSavedEvent.Remove(OnPackageSavedDelegateHandle);
#else // #if ENGINE_MAJOR_VERSION < 5
	UPackage::PackageSavedWithContextEvent.Remove(OnPackageSavedDelegateHandle);
#endif // #else // #if ENGINE_MAJOR_VERSION < 5
}

void FDialogBuilderEditor::Initialize(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UObject* InObject)
{
	UDialogBuilderGraph* DialogGraphToEdit = Cast<UDialogBuilderGraph>(InObject);

	if (DialogGraphToEdit != nullptr)
	{
		EditingDialogGraph = DialogGraphToEdit;
	}

	//Binding Functionality
	EditingDialogGraph->GetOutermost()->PackageMarkedDirtyEvent.AddRaw(this, &FDialogBuilderEditor::OnPackageMarkedDirty);

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
			DialogEditorTabFactoryPtr = GraphEditorFactory;
			DocumentManager->RegisterDocumentFactory(GraphEditorFactory);
		}
	}

	TArray<UObject*> ObjectsToEdit;
	if (EditingDialogGraph != nullptr)
	{
		ObjectsToEdit.Add(EditingDialogGraph);
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
		FDialogDefinitionsCommands::Register();
		FDialogBuilder_EditorCommands::Register();

		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

		//ToolbarBuilder->AddDialogSystemToolbar(ToolbarExtender);

		BindCommands();
		CreateInternalWidgets();
		const bool bCreateDefaultStandaloneMenu = true;
		const bool bCreateDefaultToolbar = true;
		InitAssetEditor(Mode, InitToolkitHost, DialogBuilderEditorAppName, FTabManager::FLayout::NullLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, ObjectsToEdit);

		FDialog_System_EditorModule& DialogBuilderEditorModule = FModuleManager::LoadModuleChecked<FDialog_System_EditorModule>("Dialog_System_Editor");
		AddMenuExtender(DialogBuilderEditorModule.GetMenuExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));
		AddToolbarExtender(DialogBuilderEditorModule.GetToolBarExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));

		AddApplicationMode(DialogEditorMode, MakeShareable(new FDialogBuilderEditorApplicationMode(SharedThis(this))));
		AddApplicationMode(DialogSequencerMode, MakeShareable(new FDialogBuilderSequencerApplicationMode(SharedThis(this))));


		if (DialogGraphToEdit != nullptr)
		{
			SetCurrentMode(DialogEditorMode);
		}

		if (EditingDialogGraph && EditingDialogGraph->bIsNewlyCreated)
		{
			NewDocument_OnClicked(CGT_NewDialogGraph);
			EditingDialogGraph->bIsNewlyCreated = false;
			EditingDialogGraph->Modify();
		}
		else
		{
			if (GetDialogBuilderGraph()->DialogGraphPages.Num() > 0)
			{
				OpenDocument(GetDialogBuilderGraph()->DialogGraphPages[0], FDocumentTracker::OpenNewDocument);
				
				//bind on graph changes
				for (UEdGraph* EdGraph : EditingDialogGraph->DialogGraphPages)
				{
					if (EdGraph)
					{
						if (UDialogBuilderEdGraph* DialogGraph = Cast<UDialogBuilderEdGraph>(EdGraph))
						{
							DialogGraph->AddOnGraphChangedHandler(FOnGraphChanged::FDelegate::CreateSP(this, &FDialogBuilderEditor::OnGraphChanged));
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
	RebuildDialogBuilderGraphPages();

	UpdateDialogDefinitions();
}


void FDialogBuilderEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	DocumentManager->SetTabManager(InTabManager);

	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_DialogBuilderEditor", "Generic Graph Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FDialogBuilderEditorTabs::DialogDefinitionsID, FOnSpawnTab::CreateSP(this, &FDialogBuilderEditor::SpawnTab_DialogDefinitions))
		.SetDisplayName(LOCTEXT("Dialog Details", "My Dialog"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FDialogBuilderEditorTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FDialogBuilderEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("GraphCanvasTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.EventGraph_16x"));

	InTabManager->RegisterTabSpawner(FDialogBuilderEditorTabs::DialogBuilderPropertyID, FOnSpawnTab::CreateSP(this, &FDialogBuilderEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab", "Property"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FDialogBuilderEditorTabs::DialogSequencerTabID, FOnSpawnTab::CreateSP(this, &FDialogBuilderEditor::SpawnTab_Sequencer))
		.SetDisplayName(LOCTEXT("DialogSequencerTab", "Sequencer"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Cinematics"));

	InTabManager->RegisterTabSpawner(FDialogBuilderEditorTabs::DialogSequencerViewportID, FOnSpawnTab::CreateSP(this, &FDialogBuilderEditor::SpawnTab_SequencerViewport))
		.SetDisplayName(LOCTEXT("DialogSequencerViewportTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));

}

void FDialogBuilderEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FDialogBuilderEditorTabs::DialogDefinitionsID);
	InTabManager->UnregisterTabSpawner(FDialogBuilderEditorTabs::ViewportID);
	InTabManager->UnregisterTabSpawner(FDialogBuilderEditorTabs::DialogBuilderPropertyID);
	InTabManager->UnregisterTabSpawner(FDialogBuilderEditorTabs::DialogBuilderEditorSettingsID);
	InTabManager->UnregisterTabSpawner(FDialogBuilderEditorTabs::DialogSequencerTabID);
	InTabManager->UnregisterTabSpawner(FDialogBuilderEditorTabs::DialogSequencerViewportID);
}

void FDialogBuilderEditor::Tick(float DeltaTime)
{
	PreviewScene.UpdateCaptureContents();

}

TStatId FDialogBuilderEditor::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FDialogBuilderEditor, STATGROUP_Tickables);
}

void FDialogBuilderEditor::PostUndo(bool bSuccess)
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

void FDialogBuilderEditor::PostRedo(bool bSuccess)
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

static UMovieSceneDialogTrack* FindDialogTrackForDefinition(UMovieScene* InMovieScene, const UDialogDefinition* InDialogDefinition)
{
	if (!InMovieScene || !InDialogDefinition)
	{
		return nullptr;
	}

	const UMovieScene* ConstMovieScene = InMovieScene;

	// Master tracks
	for (const UMovieSceneTrack* Track : ConstMovieScene->GetTracks())
	{
		const UMovieSceneDialogTrack* DialogTrack = Cast<UMovieSceneDialogTrack>(Track);
		if (DialogTrack && DialogTrack->DialogDefinition == InDialogDefinition)
		{
			return const_cast<UMovieSceneDialogTrack*>(DialogTrack);
		}
	}

	// Object-bound tracks
	for (const FMovieSceneBinding& Binding : ConstMovieScene->GetBindings())
	{
		for (const UMovieSceneTrack* Track : Binding.GetTracks())
		{
			const UMovieSceneDialogTrack* DialogTrack = Cast<UMovieSceneDialogTrack>(Track);
			if (DialogTrack && DialogTrack->DialogDefinition == InDialogDefinition)
			{
				return const_cast<UMovieSceneDialogTrack*>(DialogTrack);
			}
		}
	}

	return nullptr;
}



void FDialogBuilderEditor::OnDialogDefinitionAdded(UDialogDefinition* InDialogDefinition)
{
	if (!InDialogDefinition || !EditingDialogSequence)
	{
		return;
	}

	EnsureSequencerCreated();
	UpdateDialogDefinitions();

	if (Sequencer.IsValid())
	{
		Sequencer->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemsChanged);
	}
}

void FDialogBuilderEditor::OnDialogDefinitionRemoved(UDialogDefinition* InDialogDefinition)
{
	if (!InDialogDefinition || !EditingDialogSequence)
	{
		return;
	}

	EnsureSequencerCreated();
	RemoveDialogTrackDefinition(InDialogDefinition);

	if (Sequencer.IsValid())
	{
		Sequencer->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemsChanged);
	}
}
void FDialogBuilderEditor::CreateDialogTrackDefinition(UDialogDefinition* InDialogDefinition)
{
	UMovieScene* MovieScene = EditingDialogSequence ? EditingDialogSequence->GetMovieScene() : nullptr;
	if (!MovieScene || !InDialogDefinition)
	{
		return;
	}

	if (FindDialogTrackForDefinition(MovieScene, InDialogDefinition))
	{
		return;
	}

	MovieScene->Modify();

	AActor* BoundActor = DialogDefinitionActors.FindRef(InDialogDefinition).Get();

	// If we have a spawned preview actor and a valid sequencer, create an object-bound track
	if (BoundActor && Sequencer.IsValid())
	{
		const FGuid ObjectBinding = Sequencer->GetHandleToObject(BoundActor, true);
		if (ObjectBinding.IsValid())
		{
			UMovieSceneDialogTrack* DialogTrack = MovieScene->AddTrack<UMovieSceneDialogTrack>(ObjectBinding);
			if (!DialogTrack)
			{
				return;
			}

			DialogTrack->Modify();
			DialogTrack->DialogDefinition = InDialogDefinition;
			DialogTrack->SetDisplayName(FText::FromString(InDialogDefinition->GetName()));
			return;
		}
	}

	// Fallback to master track
	UMovieSceneDialogTrack* DialogTrack = MovieScene->AddTrack<UMovieSceneDialogTrack>();
	if (!DialogTrack)
	{
		return;
	}

	DialogTrack->Modify();
	DialogTrack->DialogDefinition = InDialogDefinition;
	DialogTrack->SetDisplayName(FText::FromString(InDialogDefinition->GetName()));

	
}

void FDialogBuilderEditor::RemoveDialogTrackDefinition(UDialogDefinition* InDialogDefinition)
{
	UMovieScene* MovieScene = EditingDialogSequence ? EditingDialogSequence->GetMovieScene() : nullptr;
	if (!MovieScene || !InDialogDefinition)
	{
		return;
	}
	

	UMovieSceneTrack* TracksToRemove = FindDialogTrackForDefinition(MovieScene, InDialogDefinition);


	TracksToRemove->Modify();
	MovieScene->Modify();
	MovieScene->RemoveTrack(*TracksToRemove);

	
}

void FDialogBuilderEditor::UpdateDialogDefinitions()
{
	UDialogBuilderGraph* DialogGraph = GetDialogBuilderGraph();
	check(DialogGraph);

	if (!GetPreviewScene())
	{
		return;
	}

	UWorld* PreviewWorld = GetPreviewWorld();
	check(PreviewWorld);

	AActor* PivotActor = SequencePivot.Get();
	if (!IsValid(PivotActor))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("DialogSequencePivot");
		SpawnParams.ObjectFlags = RF_Transient | RF_TextExportTransient;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		PivotActor = PreviewWorld->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
		if (PivotActor)
		{
			USceneComponent* PivotRoot = NewObject<USceneComponent>(PivotActor, TEXT("PivotRoot"));
			PivotActor->SetRootComponent(PivotRoot);
			PivotRoot->RegisterComponent();

			UBillboardComponent* BillboardComponent = NewObject<UBillboardComponent>(PivotActor, TEXT("PivotBillboard"));
			BillboardComponent->SetupAttachment(PivotRoot);
			BillboardComponent->SetMobility(EComponentMobility::Movable);
			BillboardComponent->SetHiddenInGame(true);
			BillboardComponent->SetUsingAbsoluteScale(true);
			BillboardComponent->bIsScreenSizeScaled = true;
			BillboardComponent->bUseInEditorScaling = true;
			BillboardComponent->ScreenSize = 0.0025f;
			BillboardComponent->SpriteInfo.Category = TEXT("Dialog");
			BillboardComponent->SpriteInfo.DisplayName = LOCTEXT("DialogSequencePivotSprite", "Dialog Sequence Pivot");

			if (UTexture2D* PivotSprite = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EditorResources/S_Actor.S_Actor")))
			{
				BillboardComponent->SetSprite(PivotSprite);
			}

			BillboardComponent->RegisterComponent();

			PivotActor->SetActorHiddenInGame(true);
			PivotActor->SetActorEnableCollision(false);
			PivotActor->bIsEditorOnlyActor = true;

			SequencePivot = PivotActor;
		}
	}
	PivotActor->SetActorTransform(DialogGraph->SequencePivot);

	if (!DialogPreviewCamera.IsValid())
	{
		ACineCameraActor* NewCamera = nullptr;
		const FGuid CameraGuid = CreateDialogCamera(false, NewCamera);
		if (CameraGuid.IsValid() && NewCamera)
		{
			NewCamera->AttachToActor(PivotActor, FAttachmentTransformRules::KeepRelativeTransform);
			DialogPreviewCamera = NewCamera;
		}
	}
	else if (AActor* ExistingCamera = DialogPreviewCamera.Get())
	{
		if (ExistingCamera->GetAttachParentActor() != PivotActor)
		{
			ExistingCamera->AttachToActor(PivotActor, FAttachmentTransformRules::KeepRelativeTransform);
		}
	}

	// Use DialogDefinitionActors as the single source of truth for spawned preview actors.
	// This allows removing PreviewParticipantActors completely.
	for (UDialogParticipant* DialogParticipant : DialogGraph->Participants)
	{
		if (!DialogParticipant)
		{
			continue;
		}

		const FParticipantInfo& ParticipantInfo = DialogParticipant->ParticipantInfo;
		const TSubclassOf<AActor> ParticipantClass = ParticipantInfo.ParticipantToSpawn;

		AActor* ExistingActor = nullptr;
		if (TWeakObjectPtr<AActor>* ExistingActorPtr = DialogDefinitionActors.Find(DialogParticipant))
		{
			ExistingActor = ExistingActorPtr->Get();
		}

		// If participant has no class assigned, destroy any existing preview actor and remove the track entry.
		if (!ParticipantClass)
		{
			if (ExistingActor)
			{
				ExistingActor->Destroy();
			}

			DialogDefinitionActors.Remove(DialogParticipant);
			continue;
		}

		// Reuse existing actor if it matches the desired class; otherwise respawn.
		if (ExistingActor && ExistingActor->IsA(ParticipantClass))
		{
			ExistingActor->SetActorTransform(ParticipantInfo.ParticipantTransform);
			CreateDialogTrackDefinition(DialogParticipant);
			continue;
		}

		if (ExistingActor)
		{
			ExistingActor->Destroy();
			ExistingActor = nullptr;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.bNoFail = true;
		SpawnParams.ObjectFlags = RF_Transient | RF_Transactional;

		AActor* NewActor = PreviewWorld->SpawnActor<AActor>(ParticipantClass, ParticipantInfo.ParticipantTransform, SpawnParams);
		if (NewActor)
		{
			NewActor->AttachToActor(PivotActor, FAttachmentTransformRules::KeepRelativeTransform);
			//NewActor->SetActorRelativeTransform(ParticipantInfo.ParticipantTransform);
			DialogDefinitionActors.Add(DialogParticipant, NewActor);
			CreateDialogTrackDefinition(DialogParticipant);
		}
	}

	// Cleanup: remove entries for participants that no longer exist in the dialog graph.
	for (auto It = DialogDefinitionActors.CreateIterator(); It; ++It)
	{
		const UDialogDefinition* Key = It.Key();
		if (!Key || !DialogGraph->Participants.Contains(Key))
		{
			if (AActor* Actor = It.Value().Get())
			{
				PreviewWorld->EditorDestroyActor(Actor, false);
			}
			It.RemoveCurrent();
		}
	}
}

void FDialogBuilderEditor::DestroyDialogDefinitions()
{
	UWorld* PreviewWorld = GetPreviewWorld();
	check(PreviewWorld);

	for (auto& Pair : DialogDefinitionActors)
	{
		if (AActor* Actor = Pair.Value.Get())
		{
			PreviewWorld->EditorDestroyActor(Actor, false);
		}
	}

	DialogDefinitionActors.Empty();
}

TArray<AActor*> FDialogBuilderEditor::GetDialogDefinitionActors() const
{
	TArray<AActor*> Result;
	Result.Reserve(DialogDefinitionActors.Num());

	for (const TPair<UDialogDefinition*, TWeakObjectPtr<AActor>>& Pair : DialogDefinitionActors)
	{
		if (AActor* Actor = Pair.Value.Get())
		{
			Result.Add(Actor);
		}
	}

	return Result;
}

void FDialogBuilderEditor::SelectDialogDefinitionActor(UDialogDefinition* InDialogDefinition)
{
	const bool bNoteSelectionChange = true;
	const bool bDeselectBSPSurfs = true;
	const bool bWarnAboutManyActors = false;

	AActor* ActorToSelect = DialogDefinitionActors.FindRef(InDialogDefinition).Get();
	if (GEditor)
	{
		GEditor->SelectNone(bNoteSelectionChange, bDeselectBSPSurfs, bWarnAboutManyActors);
		if (ActorToSelect)
		{
			GEditor->SelectActor(ActorToSelect, true, bNoteSelectionChange);
		}

		GEditor->NoteSelectionChange();
	}
}

void FDialogBuilderEditor::SelectDialogDefinition(UDialogDefinition* InDialogDefinition)
{
	if (!DialogDefinitionsWidget.IsValid())
	{
		return;
	}


	int32 SectionId = INDEX_NONE;
	if (Cast<UDialogParticipant>(InDialogDefinition))
	{
		SectionId = DialogSectionID::PARTICIPANTS;
	}
	else if (Cast<UDialogProp>(InDialogDefinition))
	{
		SectionId = DialogSectionID::PROPS;
	}

	DialogDefinitionsWidget->SelectItemByName(
		InDialogDefinition ? InDialogDefinition->GetFName() : NAME_None,
		ESelectInfo::Direct,
		SectionId,
		false);

	SetDetailsObject(InDialogDefinition);
	SelectDialogDefinitionActor(InDialogDefinition);
}

UDialogDefinition* FDialogBuilderEditor::FindDialogDefinitionByActor(const AActor* InActor) const
{
	if (!InActor)
	{
		return nullptr;
	}

	for (const TPair<UDialogDefinition*, TWeakObjectPtr<AActor>>& Pair : DialogDefinitionActors)
	{
		if (Pair.Key && Pair.Value.IsValid() && Pair.Value.Get() == InActor)
		{
			return Pair.Key;
		}
	}

	return nullptr;
}

UWorld* FDialogBuilderEditor::GetPreviewWorld()
{
	if (ViewportWorldMode == EDialogViewportWorldMode::CurrentLevel)
	{
		return GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	}

	return PreviewScene.GetWorld();
}


void FDialogBuilderEditor::BindCommands()
{
	ToolkitCommands->MapAction(FDialogBuilder_EditorCommands::Get().NewDialogDecorator,
		FExecuteAction::CreateSP(this, &FDialogBuilderEditor::CreateNewDialogDecorator),
		FCanExecuteAction::CreateSP(this, &FDialogBuilderEditor::CanCreateDialogDecorator)
	);

	ToolkitCommands->MapAction(FDialogBuilder_EditorCommands::Get().NewDialogEvent,
		FExecuteAction::CreateSP(this, &FDialogBuilderEditor::CreateNewDialogEvent),
		FCanExecuteAction::CreateSP(this, &FDialogBuilderEditor::CanCreateDialogEvent)
	); 
	
	ToolkitCommands->MapAction(FDialogBuilder_EditorCommands::Get().NewDialogCameraShot,
		FExecuteAction::CreateSP(this, &FDialogBuilderEditor::CreateNewDialogCameraShot),
		FCanExecuteAction::CreateSP(this, &FDialogBuilderEditor::CanCreateDialogCameraShot)
	);

	ToolkitCommands->MapAction(FDialogBuilder_EditorCommands::Get().DialogSetting,
		FExecuteAction::CreateSP(this, &FDialogBuilderEditor::OpenDialogSetting),
		FCanExecuteAction::CreateSP(this, &FDialogBuilderEditor::CanOpenDialogSetting)
	);
	
	ToolkitCommands->MapAction(FDialogBuilder_EditorCommands::Get().AddNewDialogGraph,
		FExecuteAction::CreateSP(this, &FDialogBuilderEditor::NewDocument_OnClicked, CGT_NewDialogGraph),
		FCanExecuteAction::CreateSP(this, &FDialogBuilderEditor::CanAddNewDialogGraph),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateSP(this, &FDialogBuilderEditor::NewDocument_IsVisibleForType, CGT_NewDialogGraph)
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

	GraphEditorCommands->MapAction(FDialogBuilder_EditorCommands::Get().AddNewDialogGraph,
		FExecuteAction::CreateSP(this, &FDialogBuilderEditor::NewDocument_OnClicked, CGT_NewDialogGraph),
		FCanExecuteAction::CreateSP(this, &FDialogBuilderEditor::CanAddNewDialogGraph),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateSP(this, &FDialogBuilderEditor::NewDocument_IsVisibleForType, CGT_NewDialogGraph)
	);

	GraphEditorCommands->MapAction(FDialogBuilder_EditorCommands::Get().NewDialogDecorator,
		FExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CreateNewDialogDecorator),
		FCanExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CanCreateDialogDecorator));
	
	GraphEditorCommands->MapAction(FDialogBuilder_EditorCommands::Get().NewDialogEvent,
		FExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CreateNewDialogEvent),
		FCanExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CanCreateDialogEvent));

	GraphEditorCommands->MapAction(FDialogBuilder_EditorCommands::Get().DialogSetting,
		FExecuteAction::CreateRaw(this, &FDialogBuilderEditor::OpenDialogSetting),
		FCanExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CanOpenDialogSetting));

	GraphEditorCommands->MapAction(FDialogBuilder_EditorCommands::Get().NewDialogCameraShot,
		FExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CreateNewDialogCameraShot),
		FCanExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CanCreateDialogCameraShot));

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

	GraphEditorCommands->MapAction(
		FGraphEditorCommands::Get().CreateComment,
		FExecuteAction::CreateRaw(this, &FDialogBuilderEditor::OnCreateComment),
		FCanExecuteAction::CreateRaw(this, &FDialogBuilderEditor::CanCreateComment)
	);
}

TSharedPtr<SGraphEditor> FDialogBuilderEditor::GetCurrGraphEditor() const
{
	return CurrentGraphWidget;
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

const FSlateBrush* FDialogBuilderEditor::GetDefaultTabIcon() const
{
	return FDialogBuilder_EditorStyle::Get().GetBrush("ClassIcon.DialogNode");
}

FName FDialogBuilderEditor::GetToolkitFName() const
{
	return FName("FDialogGraphEditor");
}

FText FDialogBuilderEditor::GetBaseToolkitName() const
{
	return LOCTEXT("DialogGraphEditorAppLabel", "Dialog Graph Editor");
}


FText FDialogBuilderEditor::GetToolkitName() const
{
	const bool bDirtyState = EditingDialogGraph->GetOutermost()->IsDirty();

	FFormatNamedArguments Args;
	Args.Add(TEXT("DialogGraphName"), FText::FromString(EditingDialogGraph->GetName()));
	Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
	return FText::Format(LOCTEXT("DialogGraphEditorToolkitName", "{DialogGraphName}"), Args);
}

FText FDialogBuilderEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(EditingDialogGraph);
}

FLinearColor FDialogBuilderEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

FString FDialogBuilderEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("DialogGraphEditor");
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

void FDialogBuilderEditor::RefreshDialogDefinitions()
{
}

void FDialogBuilderEditor::RefreshInspector()
{
}

void FDialogBuilderEditor::AddToSelection(UEdGraphNode* InNode)
{
}

//Sequencer
void FDialogBuilderEditor::OpenDialogSequence(UDialogSequence* InDialogSequence)
{
	if (!InDialogSequence)
	{
		return;
	}

	EditingDialogSequence = InDialogSequence;

	Sequencer.Reset();

	EnsureSequencerCreated();

	// Switch to sequencer mode and bring up the tabs
	SetCurrentMode(DialogSequencerMode);

}


void FDialogBuilderEditor::ExtendSequencerToolbar(FName InToolMenuName)
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* ToolMenu = UToolMenus::Get()->ExtendMenu(InToolMenuName);

	const FToolMenuInsert SectionInsertLocation("BaseCommands", EToolMenuInsertType::Before);

	{
		ToolMenu->AddDynamicSection("LevelSequenceEditorDynamic", FNewToolMenuDelegate::CreateLambda([](UToolMenu* InMenu)
			{
				UDialogSequenceEditorMenuContext* DialogSequenceEditorMenuContext = InMenu->FindContext<UDialogSequenceEditorMenuContext>();
				if (DialogSequenceEditorMenuContext && DialogSequenceEditorMenuContext->DialogEditor.IsValid())
				{
					const FName SequencerToolbarStyleName = "SequencerToolbar";

					FToolMenuEntry PlaybackContextEntry = FToolMenuEntry::InitWidget(
						"PlaybackContext",
						DialogSequenceEditorMenuContext->DialogEditor.Pin()->PlaybackContext->BuildWorldPickerCombo(),
						LOCTEXT("PlaybackContext", "PlaybackContext")
					);
					PlaybackContextEntry.StyleNameOverride = SequencerToolbarStyleName;

					FToolMenuSection& Section = InMenu->AddSection("LevelSequenceEditor");
					Section.AddEntry(PlaybackContextEntry);
				}
			}), SectionInsertLocation);
	}
}


void FDialogBuilderEditor::OnSequencerReceivedFocus()
{
	if (Sequencer.IsValid())
	{
		//FLevelEditorSequencerIntegration::Get().OnSequencerReceivedFocus(Sequencer.ToSharedRef());
	}
}

void FDialogBuilderEditor::OnInitToolMenuContext(FToolMenuContext& MenuContext)
{
	UDialogSequenceEditorMenuContext* DialogSequenceEditorMenuContext = NewObject<UDialogSequenceEditorMenuContext>();
	DialogSequenceEditorMenuContext->DialogEditor = SharedThis(this);
	MenuContext.AddObject(DialogSequenceEditorMenuContext);
}

void FDialogBuilderEditor::EnsureSequencerCreated()
{
	if (Sequencer.IsValid())
	{
		return;
	}

	if (!EditingDialogSequence)
	{
		return;
	}

	PlaybackContext = MakeShared<FLevelSequencePlaybackContext>(EditingDialogSequence);

	TSharedRef<FLevelSequenceEditorSpawnRegister> SpawnRegister = MakeShareable(new FLevelSequenceEditorSpawnRegister);
	
	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer");
	
	FSequencerViewParams ViewParams(TEXT("DialogSequenceSetting"));
	{
		ViewParams.UniqueName = "DialogSequenceEditor";
		ViewParams.ScrubberStyle = ESequencerScrubberStyle::FrameBlock;
		ViewParams.OnReceivedFocus.BindRaw(this, &FDialogBuilderEditor::OnSequencerReceivedFocus);
		ViewParams.OnInitToolMenuContext.BindRaw(this, &FDialogBuilderEditor::OnInitToolMenuContext);
		//ViewParams.ToolbarExtender = FToolBarExtender::CreateRaw(this, &FDialogBuilderEditor::ExtendSequencerToolbar);
	}


	FSequencerInitParams InitParams;
	{
		InitParams.RootSequence = EditingDialogSequence;
		InitParams.bEditWithinLevelEditor = false;
		InitParams.ToolkitHost = GetToolkitHost();
		InitParams.SpawnRegister = SpawnRegister;


		InitParams.EventContexts.Bind(PlaybackContext.ToSharedRef(), &FLevelSequencePlaybackContext::GetEventContexts);
		InitParams.PlaybackContext.Bind(PlaybackContext.ToSharedRef(), &FLevelSequencePlaybackContext::GetPlaybackContextAsObject);
		InitParams.PlaybackClient.Bind(PlaybackContext.ToSharedRef(), &FLevelSequencePlaybackContext::GetPlaybackClientAsInterface);

		InitParams.ViewParams = ViewParams;

		InitParams.HostCapabilities.bSupportsCurveEditor = true;
		InitParams.HostCapabilities.bSupportsSaveMovieSceneAsset = true;
		InitParams.HostCapabilities.bSupportsRecording = true;
		InitParams.HostCapabilities.bSupportsRenderMovie = true;
		InitParams.HostCapabilities.bSupportsAddFromContentBrowser = true;
		InitParams.HostCapabilities.bSupportsSidebar = true;
		InitParams.HostCapabilities.bSupportsViewportSelectability = true;
	}

	ExtendSequencerToolbar("DialogSequence.MainToolbar");

	Sequencer = SequencerModule.CreateSequencer(InitParams);

	SpawnRegister->SetSequencer(Sequencer);

}

void NewCameraAdded(TSharedRef<ISequencer> Sequencer, ACameraActor* NewCamera, FGuid CameraGuid)
{
	if (Sequencer->OnCameraAddedToSequencer().IsBound() && !Sequencer->OnCameraAddedToSequencer().Execute(NewCamera, CameraGuid))
	{
		return;
	}

	MovieSceneToolHelpers::LockCameraActorToViewport(Sequencer, NewCamera);

	UMovieSceneSequence* Sequence = Sequencer->GetFocusedMovieSceneSequence();
	if (Sequence && Sequence->IsTrackSupported(UMovieSceneCameraCutTrack::StaticClass()) == ETrackSupport::Supported)
	{
		MovieSceneToolHelpers::CreateCameraCutSectionForCamera(Sequence->GetMovieScene(), CameraGuid, Sequencer->GetLocalTime().Time.FloorToFrame());
	}
}

FGuid FDialogBuilderEditor::CreateDialogCamera(const bool bSpawnable, ACineCameraActor*& OutActor)
{
	FGuid CameraGuid;

	if (!Sequencer.IsValid() || !EditingDialogSequence)
	{
		return CameraGuid;
	}

	UMovieSceneSequence* Sequence = Sequencer->GetFocusedMovieSceneSequence();
	if (!Sequence)
	{
		return CameraGuid;
	}

	UMovieScene* MovieScene = Sequence->GetMovieScene();
	if (!MovieScene || MovieScene->IsReadOnly())
	{
		return CameraGuid;
	}

	UWorld* World = GetPreviewWorld();
	if (!World)
	{
		return CameraGuid;
	}

	const FScopedTransaction Transaction(LOCTEXT("CreateDialogCamera", "Create Dialog Camera"));

	FActorSpawnParameters SpawnParams;
	if (bSpawnable)
	{
		SpawnParams.ObjectFlags &= ~RF_Transactional;
	}
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	OutActor = World->SpawnActor<ACineCameraActor>(SpawnParams);
	if (!OutActor)
	{
		return CameraGuid;
	}


	//OutActor->SetActorLocation(GCurrentLevelEditingViewportClient->GetViewLocation(), false);
	//OutActor->SetActorRotation(GCurrentLevelEditingViewportClient->GetViewRotation());

	OutActor->SetActorLocation(FVector::ZeroVector, false);
	OutActor->SetActorRotation(FRotator::ZeroRotator);
	FActorLabelUtilities::SetActorLabelUnique(OutActor, ACineCameraActor::StaticClass()->GetName());

	CameraGuid = FSequencerUtilities::CreateBinding(Sequencer.ToSharedRef(), *OutActor);

	/*TSubclassOf<UMovieSceneCustomBinding> CustomBindingClass = bSpawnable
		? UMovieSceneSpawnableActorBinding::StaticClass()
		: UMovieSceneReplaceableActorBinding::StaticClass();

	const FMovieSceneBindingReferences* BindingReferences = Sequence->GetBindingReferences();
	if (BindingReferences)
	{
		for (const FMovieSceneBindingReference& Reference : BindingReferences->GetReferences(CameraGuid))
		{
			for (const TSubclassOf<UMovieSceneCustomBinding>& SupportedCustomBindingType : Sequencer->GetSupportedCustomBindingTypes())
			{
				if (SupportedCustomBindingType
					&& SupportedCustomBindingType->IsChildOf(CustomBindingClass)
					&& SupportedCustomBindingType->GetDefaultObject<UMovieSceneCustomBinding>()->SupportsConversionFromBinding(Reference, OutActor))
				{
					FMovieScenePossessable* NewPossessable = FSequencerUtilities::ConvertToCustomBinding(Sequencer.ToSharedRef(), CameraGuid, CustomBindingClass);
					if (NewPossessable)
					{
						for (TWeakObjectPtr<> WeakObject : Sequencer->FindBoundObjects(NewPossessable->GetGuid(), Sequencer->GetFocusedTemplateID()))
						{
							if (ACineCameraActor* SpawnedActor = Cast<ACineCameraActor>(WeakObject.Get()))
							{
								OutActor = SpawnedActor;
							}
						}

						CameraGuid = NewPossessable->GetGuid();
					}
					break;
				}
			}
		}
	}*/

	if (!CameraGuid.IsValid())
	{
		return CameraGuid;
	}

	NewCameraAdded(Sequencer.ToSharedRef(), OutActor, CameraGuid);

	return CameraGuid;
}



TSharedRef<SDockTab> FDialogBuilderEditor::SpawnTab_Sequencer(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FDialogBuilderEditorTabs::DialogSequencerTabID);

	EnsureSequencerCreated();

	return SNew(SDockTab)
		.Label(LOCTEXT("DialogSequencerTab_Title", "Sequencer"))
		[
			Sequencer.IsValid()
				? Sequencer->GetSequencerWidget()
				: SNullWidget::NullWidget
		];
}

TSharedRef<SDockTab> FDialogBuilderEditor::SpawnTab_SequencerViewport(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FDialogBuilderEditorTabs::DialogSequencerViewportID);

	return SNew(SDockTab)
		.Label(LOCTEXT("DialogSequencerViewport_Title", "Viewport"))
		[
			SequencerViewportWidget.IsValid()
				? SequencerViewportWidget.ToSharedRef()
				: SNullWidget::NullWidget
		];
}

void FDialogBuilderEditor::JumpToHyperlink(const UObject* ObjectReference, bool bRedialogRename)
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

	Collector.AddReferencedObject(EditingDialogGraph);*/
}

FString FDialogBuilderEditor::GetReferencerName() const
{
	return FString();
}

UDialogBuilderGraph* FDialogBuilderEditor::GetDialogBuilderGraph() const
{
	return EditingDialogGraph;
}

void FDialogBuilderEditor::RestoreDialogEditor()
{
	// Update dialog asset data based on saved graph to have correct data in editor
	TWeakObjectPtr< UEdGraph > FocusedGraphPtr = EditingDialogGraph->DialogGraphPages.Num() > 0 ? EditingDialogGraph->DialogGraphPages[0] : nullptr;
	UDialogBuilderEdGraph* MyGraph = Cast<UDialogBuilderEdGraph>(FocusedGraphPtr.Get());
	const bool bNewGraph = MyGraph == NULL;

	TSharedRef<FTabPayload_UObject> Payload = FTabPayload_UObject::Make(MyGraph);
	TSharedPtr<SDockTab> DocumentTab = DocumentManager->OpenDocument(Payload, bNewGraph ? FDocumentTracker::OpenNewDocument : FDocumentTracker::RestorePreviousDocument);

		
	

	if (EditingDialogGraph->LastEditedDocuments.Num() > 0 && DocumentTab.IsValid())
	{
		TSharedPtr<SWidget> Content = DocumentTab->GetContent();
		if (Content.IsValid())
		{
			TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Content.ToSharedRef());
			GraphEditor->SetViewLocation(
				EditingDialogGraph->LastEditedDocuments[0].SavedViewOffset,
				EditingDialogGraph->LastEditedDocuments[0].SavedZoomAmount
			);
		}
	}

}

void FDialogBuilderEditor::SaveEditedObjectState()
{
	// Clear currently edited documents
	EditingDialogGraph->LastEditedDocuments.Empty();

	// Ask all open documents to save their state, which will update LastEditedDocuments
	DocumentManager->SaveAllState();
}

bool FDialogBuilderEditor::NewDocument_IsVisibleForType(ECreatedDialogDocumentType GraphType) const
{
	return false;
}

void FDialogBuilderEditor::NewDocument_OnClicked(ECreatedDialogDocumentType GraphType)
{
	FText DocumentNameText;
	bool bResetMyBlueprintFilter = false;

	switch (GraphType)
	{
	case CGT_NewDialogGraph:
		DocumentNameText = LOCTEXT("NewDocDialogName", "Dialog Graph");
		bResetMyBlueprintFilter = true;
		break;
	
	default:
		DocumentNameText = LOCTEXT("NewDocNewName", "NewDocument");
		break;
	}

	FName DocumentName = FName(*DocumentNameText.ToString());

	

	// Make sure the new name is valid
	DocumentName = FDialogBuilderEditorUtils::FindUniqueDialogName(DocumentNameText.ToString());
		
	//check(IsEditingSingleBlueprint());

	const FScopedTransaction Transaction(LOCTEXT("AddNewDialogGraph", "Add New Dialog Graph"));
	GetDialogBuilderGraph()->Modify();

	UEdGraph* NewGraph = nullptr;

	
	if (GraphType == CGT_NewDialogGraph)
	{
		NewGraph = FDialogBuilderEditorUtils::CreateNewGraph(GetDialogBuilderGraph(), DocumentName, UDialogBuilderEdGraph::StaticClass(), UEdGraphSchema_DialogBuilder::StaticClass());
		NewGraph->AddOnGraphChangedHandler(FOnGraphChanged::FDelegate::CreateSP(this, &FDialogBuilderEditor::OnGraphChanged));
		FDialogBuilderEditorUtils::AddDialogGraphPage(GetDialogBuilderGraph(), NewGraph);

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

bool FDialogBuilderEditor::CanAddNewDialogGraph() const
{
	return false;
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
	if (DialogDefinitionsWidget.IsValid())
	{
		// Force a refresh immediately, the item has to be present in the list for the rename redialogs to be successful.
		DialogDefinitionsWidget->Refresh();
		DialogDefinitionsWidget->SelectItemByName(InActionName, ESelectInfo::OnMouseClick);
		DialogDefinitionsWidget->OnRequestRenameOnActionNode();
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
	check(IsEditingSingleDialogGraph());

	UDialogBuilderGraph* DialogBuilderGraph = GetDialogBuilderGraph();
	if (DialogBuilderGraph->LastEditedDocuments.Num() == 0)
	{
			DialogBuilderGraph->LastEditedDocuments.Add(FDialogBuilderEditorUtils::FindDialogGraph(DialogBuilderGraph));
	}

	for (int32 i = 0; i < DialogBuilderGraph->LastEditedDocuments.Num(); i++)
	{
		if (UObject* Obj = DialogBuilderGraph->LastEditedDocuments[i].EditedObjectPath.ResolveObject())
		{
			if (UEdGraph* Graph = Cast<UEdGraph>(Obj))
			{
				struct LocalStruct
				{
					static TSharedPtr<SDockTab> OpenGraphTree(FDialogBuilderEditor* InDialogSystemGraphEditor, UEdGraph* InGraph)
					{
						FDocumentTracker::EOpenDocumentCause OpenCause = FDocumentTracker::QuickNavigateCurrentDocument;

						for (UObject* OuterObject = InGraph->GetOuter(); OuterObject; OuterObject = OuterObject->GetOuter())
						{
							if (OuterObject->IsA<UDialogBuilderGraph>())
							{
								// reached up to the DialogBuilderGraph for the graph, we are done climbing the tree
								OpenCause = FDocumentTracker::RestorePreviousDocument;
								break;
							}
							else if (UEdGraph* OuterGraph = Cast<UEdGraph>(OuterObject))
							{
								// Found another graph, open it up
								OpenGraphTree(InDialogSystemGraphEditor, OuterGraph);
								break;
							}
						}

						return InDialogSystemGraphEditor->OpenDocument(InGraph, OpenCause);
					}
				};
				TSharedPtr<SDockTab> TabWithGraph = LocalStruct::OpenGraphTree(this, Graph);
				if (TabWithGraph.IsValid())
				{
					TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(TabWithGraph->GetContent());
					GraphEditor->SetViewLocation(DialogBuilderGraph->LastEditedDocuments[i].SavedViewOffset, DialogBuilderGraph->LastEditedDocuments[i].SavedZoomAmount);
				}
			}
			else
			{
				TSharedPtr<SDockTab> TabWithGraph = OpenDocument(Obj, FDocumentTracker::RestorePreviousDocument);
			}
		}
	}
}



bool FDialogBuilderEditor::IsEditingSingleDialogGraph() const
{
	return GetDialogBuilderGraph() != nullptr;
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

bool FDialogBuilderEditor::CanAccessDialogEditorMode() const
{
	return true;
}

bool FDialogBuilderEditor::CanAccessDialogSequencerMode() const
{
	return true;
}

FText FDialogBuilderEditor::GetLocalizedMode(FName InMode)
{
	static TMap< FName, FText > LocModes;

	if (LocModes.Num() == 0)
	{
		LocModes.Add(DialogEditorMode, DialogEditorModeText);
		LocModes.Add(DialogSequencerMode,DialogSequencerModeText);
	}

	check(InMode != NAME_None);
	const FText* OutDesc = LocModes.Find(InMode);
	check(OutDesc);
	return *OutDesc;
}

UEdGraphNode* FDialogBuilderEditor::GetSingleSelectedNode() const
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	return (SelectedNodes.Num() == 1) ? Cast<UEdGraphNode>(*SelectedNodes.CreateConstIterator()) : nullptr;
}

void FDialogBuilderEditor::OnGraphEditorFocused(const TSharedRef<class SGraphEditor>& InGraphEditor)
{
	// Update the graph editor that is currently focused
	CurrentGraphWidget = InGraphEditor;
	InGraphEditor->SetPinVisibility(SGraphEditor::EPinVisibility::Pin_Show);

	// Update the inspector as well, to show selection from the focused graph editor
	FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	//FocusInspectorOnGraphSelection(SelectedNodes, /*bForceRefresh=*/ true);

	// During undo, garbage graphs can be temporarily brought into focus, ensure that before a refresh of the MyBlueprint window that the graph is owned by a Blueprint
	if (CurrentGraphWidget.IsValid() && DialogDefinitionsWidget.IsValid())
	{
		// The focused graph can be garbage as well
		TWeakObjectPtr< UEdGraph > FocusedGraphPtr = CurrentGraphWidget->GetCurrentGraph();
		UEdGraph* FocusedGraph = FocusedGraphPtr.Get();
		
		if (FocusedGraph != nullptr)
		{
			if (UDialogBuilderEdGraph* DialogEdGraph = Cast<UDialogBuilderEdGraph>(FocusedGraph))
			{
				DialogEdGraph->SEditorGraph = CurrentGraphWidget.Get();
			}
			DialogDefinitionsWidget->Refresh();
		}
	}

	
}

void FDialogBuilderEditor::OnGraphEditorBackgrounded(const TSharedRef<SGraphEditor>& InGraphEditor)
{
}

bool FDialogBuilderEditor::IsGraphInCurrentDialogGraph(const UEdGraph* InGraph) const
{
	bool bEditable = true;

	UDialogBuilderGraph* EditingBP = GetDialogBuilderGraph();
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
		if (GetDialogBuilderGraph()->DialogComponent)
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

	if (CurrentGraphWidget.IsValid())
	{
		SpawnedTab->SetContent(CurrentGraphWidget.ToSharedRef());
	}

	return SpawnedTab;
}

TSharedRef<SDockTab> FDialogBuilderEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FDialogBuilderEditorTabs::DialogBuilderPropertyID);

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
	check(Args.GetTabId() == FDialogBuilderEditorTabs::DialogBuilderEditorSettingsID);

	return SNew(SDockTab)
#if ENGINE_MAJOR_VERSION < 5
		.Icon(FAppStyle::GetBrush("LevelEditor.Tabs.Details"))
#endif // #if ENGINE_MAJOR_VERSION < 5
		.Label(LOCTEXT("EditorSettings_Title", "Generic Graph Editor Setttings"))
		[
			EditorSettingsWidget.ToSharedRef()
		];
}

TSharedRef<SDockTab> FDialogBuilderEditor::SpawnTab_DialogDefinitions(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FDialogBuilderEditorTabs::DialogDefinitionsID);

	return SNew(SDockTab)
#if ENGINE_MAJOR_VERSION < 5
		.Icon(FAppStyle::GetBrush("LevelEditor.Tabs.Details"))
#endif // #if ENGINE_MAJOR_VERSION < 5
		.Label(LOCTEXT("DialogDefinitions_Title", "My Dialog"))
		[
			DialogDefinitionsWidget.ToSharedRef()
		];
}

void FDialogBuilderEditor::CreateInternalWidgets()
{
	//create details view
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.NotifyHook = this;
	DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;	

	PropertyWidget = PropertyModule.CreateDetailView(DetailsViewArgs);
	PropertyWidget->SetObject( EditingDialogGraph );
	PropertyWidget->OnFinishedChangingProperties().AddSP(this, &FDialogBuilderEditor::OnFinishedChangingProperties);


	this->DialogDefinitionsWidget = SNew(SDialogDefinitions, SharedThis(this));

	if (!SequencerViewportWidget.IsValid())
	{
		SequencerViewportWidget = SNew(SDialogPreviewViewport, SharedThis(this));
		SequencerViewportWidget->SetUseLevelWorld(ViewportWorldMode == EDialogViewportWorldMode::CurrentLevel);
	}
}


TSharedRef<SWidget> FDialogBuilderEditor::SpawnProperties()
{
	return
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.HAlign(HAlign_Fill)
		[
			PropertyWidget.ToSharedRef()
		];
		
}

TSharedRef<SWidget> FDialogBuilderEditor::SpawnDialogDefinitions()
{
	return DialogDefinitionsWidget.ToSharedRef();
}

TSharedRef<SWidget> FDialogBuilderEditor::SpawnDialogSequencerTab()
{
	EnsureSequencerCreated();

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.HAlign(HAlign_Fill)
		[
			Sequencer.IsValid()
				? Sequencer->GetSequencerWidget()
				: SNullWidget::NullWidget
		];
}

TSharedRef<SWidget> FDialogBuilderEditor::SpawnDialogSequencerViewportTab()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.HAlign(HAlign_Fill)
		[
			SequencerViewportWidget.IsValid()
				? SequencerViewportWidget.ToSharedRef()
				: SNullWidget::NullWidget
		];
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


void FDialogBuilderEditor::RebuildDialogBuilderGraphPages()
{
	if (EditingDialogGraph == nullptr)
	{
		return;
	}

	for (UEdGraph* EdGraph : EditingDialogGraph->DialogGraphPages)
	{
		if (EdGraph)
		{
			if (UDialogBuilderEdGraph* DialogGraph = Cast<UDialogBuilderEdGraph>(EdGraph))
			{
				DialogGraph->UpdateAsset();
			}
		}
	}
}

	



void FDialogBuilderEditor::OnGraphChanged(const FEdGraphEditAction& Action)
{
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

		if (UDialogBuilderEdNode* DialogEdNode = Cast<UDialogBuilderEdNode>(EdNode))
		{
			DialogEdNode->Modify();

			const UEdGraphSchema* Schema = DialogEdNode->GetSchema();
			if (Schema != nullptr)
			{
				Schema->BreakNodeLinks(*DialogEdNode);
			}

			DialogEdNode->DestroyNode();
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
	TArray<UDialogBuilderEdNode*> SubNodes;

	FString ExportedText;

	ShouldGetNewID = true;

	int32 CopySubNodeIndex = 0;
	for (FGraphPanelSelectionSet::TIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		UDialogBuilderEdNode* DialogEdNode = Cast<UDialogBuilderEdNode>(Node);
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

		if (DialogEdNode)
		{
			DialogEdNode->CopySubNodeIndex = CopySubNodeIndex;

			// append all subnodes for selection
			for (int32 Idx = 0; Idx < DialogEdNode->SubNodes.Num(); Idx++)
			{
				DialogEdNode->SubNodes[Idx]->CopySubNodeIndex = CopySubNodeIndex;
				SubNodes.Add(DialogEdNode->SubNodes[Idx]);
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
		UDialogBuilderEdNode* Node = Cast<UDialogBuilderEdNode>(*SelectedIter);
		if (Node)
		{
			Node->PostCopyNode();
		}
	}
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
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
		FVector2D PasteLocation2D = FVector2D(CurrentGraphEditor->GetPasteLocation2f().X, CurrentGraphEditor->GetPasteLocation2f().Y);
		PasteNodesHere(CurrentGraphEditor->GetCurrentGraph(), PasteLocation2D);
#elif (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 5)
		PasteNodesHere(CurrentGraphEditor->GetCurrentGraph(), CurrentGraphEditor->GetPasteLocation());
#endif
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
		// Undo/Redo support
		const FScopedTransaction Transaction(FGenericCommands::Get().Paste->GetDescription());
		UDialogBuilderEdGraph* DialogEdGraph = Cast<UDialogBuilderEdGraph>(EdGraph);

		EdGraph->Modify();

		if (DialogEdGraph)
		{
			DialogEdGraph->LockUpdates();
		}

		UDialogBuilderEdNode* SelectedParent = NULL;
		bool bHasMultipleNodesSelected = false;

		const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
		for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
		{
			UDialogBuilderEdNode* Node = Cast<UDialogBuilderEdNode>(*SelectedIter);
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
			UDialogBuilderEdNode* DialogEdNode = Cast<UDialogBuilderEdNode>(EdNode);
			if (EdNode && (DialogEdNode == nullptr || !DialogEdNode->IsSubNode()))
			{
				AvgNodePosition.X += EdNode->NodePosX;
				AvgNodePosition.Y += EdNode->NodePosY;
				++AvgCount;
			}
			
			if (ShouldGetNewID && DialogEdNode && !DialogEdNode->IsSubNode())
			{
				UDialogBuilderNode* DialogNode = DialogEdNode ? Cast<UDialogBuilderNode>(DialogEdNode->NodeInstance) : nullptr;
				if (DialogNode)
				{
					DialogEdNode->FindUniqueNodeName(DialogNode->ID.ToString());
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

		TMap<int32, UDialogBuilderEdNode*> ParentMap;
		for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
		{
			UEdGraphNode* PasteNode = *It;
			UDialogBuilderEdNode* PasteDialogEdNode = Cast<UDialogBuilderEdNode>(PasteNode);

			if (PasteNode && (PasteDialogEdNode == nullptr || !PasteDialogEdNode->IsSubNode()))
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

				if (PasteDialogEdNode)
				{
					PasteDialogEdNode->RemoveAllSubNodes();
					ParentMap.Add(PasteDialogEdNode->CopySubNodeIndex, PasteDialogEdNode);
				}
			}
		}

		for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
		{
			UDialogBuilderEdNode* PasteNode = Cast<UDialogBuilderEdNode>(*It);
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

		if (DialogEdGraph)
		{
			DialogEdGraph->UpdateClassData();
			DialogEdGraph->UnlockUpdates();
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


void FDialogBuilderEditor::FixupPastedNodes(const TSet<UEdGraphNode*>& NewPastedGraphNodes, const TMap<FGuid, FGuid>& NewToOldNodeMapping)
{
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

void FDialogBuilderEditor::HandleNewNodeClassPicked(UClass* InClass) const
{

	if (EditingDialogGraph != nullptr && InClass != nullptr && EditingDialogGraph->GetOutermost())
	{
		const FString ClassName = FBlueprintEditorUtils::GetClassNameWithoutSuffix(InClass);

		FString PathName = EditingDialogGraph->GetOutermost()->GetPathName();
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

void FDialogBuilderEditor::CreateNewDialogDecorator()
{
	HandleNewNodeClassPicked(UOrionDecorator::StaticClass());
}
bool FDialogBuilderEditor::CanCreateDialogDecorator() const
{
	return true;
}
void FDialogBuilderEditor::CreateNewDialogEvent()
{
	HandleNewNodeClassPicked(UOrionEvent::StaticClass());
}
bool FDialogBuilderEditor::CanCreateDialogEvent() const
{
	return true;
}
void FDialogBuilderEditor::CreateNewDialogCameraShot()
{
	HandleNewNodeClassPicked(UDialogCameraShot::StaticClass());
}
bool FDialogBuilderEditor::CanCreateDialogCameraShot() const
{
	return true;
}
void FDialogBuilderEditor::OpenDialogSetting()
{
	PropertyWidget->SetObject(GetDialogBuilderGraph());
}
bool FDialogBuilderEditor::CanOpenDialogSetting() const
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

bool FDialogBuilderEditor::CanCreateComment() const
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	return CurrentGraphEditor.IsValid();
}

void FDialogBuilderEditor::OnCreateComment()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrGraphEditor();
	if (UEdGraph* EdGraph = CurrentGraphEditor.IsValid() ? CurrentGraphEditor->GetCurrentGraph() : nullptr)
	{
		TSharedPtr<FEdGraphSchemaAction> Action = EdGraph->GetSchema()->GetCreateCommentAction();
		if (Action.IsValid())
		{
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
			Action->PerformAction(EdGraph, nullptr, FVector2f());
#elif (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 5)
			Action->PerformAction(EdGraph, nullptr, FVector2D());
#endif
		}
	}
}

void FDialogBuilderEditor::SetViewportWorldMode(EDialogViewportWorldMode NewMode)
{
	if (ViewportWorldMode == NewMode)
	{
		return;
	}

	ViewportWorldMode = NewMode;

	DestroyDialogDefinitions();

	if (SequencerViewportWidget.IsValid())
	{
		SequencerViewportWidget->SetUseLevelWorld(ViewportWorldMode == EDialogViewportWorldMode::CurrentLevel);
	}

	UpdateDialogDefinitions();

}

void FDialogBuilderEditor::SetDetailsObject(UObject* InObject)
{
	if (!PropertyWidget.IsValid())
	{
		return;
	}

	PropertyWidget->SetObject(InObject);
}

void FDialogBuilderEditor::OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection)
{
	TArray<UObject*> Selection = FDialogBuilderEditorUtils::GetSelectionForPropertyEditor(NewSelection);

	if (Selection.Num() == 0)
	{
		if (UDialogBuilderEdGraph* DialogGraph = Cast<UDialogBuilderEdGraph>(CurrentGraphWidget->GetCurrentGraph()))
		{
			PropertyWidget->SetObject(DialogGraph->OwningDialog);
		}
	}
	else if (Selection.Num() == 1)
	{
		if (UDialogBuilderNode_Root* RootNode = Cast<UDialogBuilderNode_Root>(Selection[0]))
		{
			if (UDialogBuilderEdGraph* DialogGraph = Cast<UDialogBuilderEdGraph>(CurrentGraphWidget->GetCurrentGraph()))
			{
				PropertyWidget->SetObject(DialogGraph->OwningDialog);
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
	UDialogBuilderEdNode* DialogEdNode = Cast<UDialogBuilderEdNode>(Node);

	if(UDialogBuilderNode_DialogSequence* DialogSequenceNode = Cast<UDialogBuilderNode_DialogSequence>(DialogEdNode ? DialogEdNode->NodeInstance : nullptr))
	{

		OpenDialogSequence(DialogSequenceNode->DialogSequence);
	}

	


	if (DialogEdNode && 
		DialogEdNode->NodeInstance->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UClass* NodeClass = DialogEdNode->NodeInstance->GetClass();
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
	if (EditingDialogGraph == nullptr)
		return;

	for (UEdGraph* EdGraph : EditingDialogGraph->DialogGraphPages)
	{
		EdGraph->GetSchema()->ForceVisualizationCacheClear();
	}
	DocumentManager->RefreshAllTabs();

}
#include "Async/Async.h"

void FDialogBuilderEditor::OnPackageMarkedDirty(UPackage* ModifiedPackage, bool bWasDirty)
{   
}
//Called when saving our file graph
#if ENGINE_MAJOR_VERSION < 5

void FDialogBuilderEditor::OnPackageSaved(const FString& PackageFileName, UObject* Outer)
{
	RebuildDialogBuilderGraphPages();
}
#else // #if ENGINE_MAJOR_VERSION < 5
void FDialogBuilderEditor::OnPackageSavedWithContext(const FString& PackageFileName, UPackage* Package, FObjectPostSaveContext ObjectSaveContext)
{
	RebuildDialogBuilderGraphPages();
	
}


#endif // #else // #if ENGINE_MAJOR_VERSION < 5

#undef LOCTEXT_NAMESPACE
