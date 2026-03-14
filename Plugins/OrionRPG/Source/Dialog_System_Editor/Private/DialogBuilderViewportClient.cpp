#include "DialogBuilderViewportClient.h"

#include "DialogBuilderEditor.h"
#include "DialogBuilderSetting.h"

#include "AdvancedPreviewScene.h"
#include "Editor.h"
#include "EditorViewportClient.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "SNameComboBox.h"
#include "ThumbnailRendering/SceneThumbnailInfo.h"
#include "ToolMenus.h"
#include "Viewports.h"
#include "EditorViewportSelectability.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Selection.h"
#include "EngineUtils.h"
#include "HitProxies.h"
#include "ComponentVisualizer.h"
#include "Styling/AppStyle.h"
#include "SnappingUtils.h"
#include "EditorModeManager.h"
#include "PreviewScene.h"
#include "Engine/RendererSettings.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "EditorModeManager.h"
#include "UnrealWidget.h"
#include "ScopedTransaction.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "AdvancedPreviewSceneMenus.h"
#include "ViewportToolbar/UnrealEdViewportToolbar.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

FDialogBuilderViewportClient::FDialogBuilderViewportClient(
	FPreviewScene* InPreviewScene,
	const TSharedRef<SEditorViewport>& InViewport,
	const TSharedRef<FDialogBuilderEditor>& InEditor,
	bool bUseLevelWorld)
	: FEditorViewportClient(nullptr, InPreviewScene, InViewport)
	, ScopedTransaction(nullptr)
	, Editor(InEditor)
	, WidgetMode(UE::Widget::WM_Translate)
	, WidgetCoordSystem(COORD_Local)
	, CurrentPreviewScene(InPreviewScene)
{
	ViewportType = LVT_Perspective;
	SetViewLocation(FVector(-250.0, 0.0, 125.0));
	SetViewRotation(FRotator(0.0, 0.0, 0.0));
	SetRealtime(true);
	bSetListenerPosition = false;

	check(Widget);
	Widget->SetSnapEnabled(true);

	// Set if the grid will be drawn
	DrawHelper.bDrawGrid = GetDefault<UDialogBuilderSetting>()->bShowGrid;

	EngineShowFlags.SetSelectionOutline(true);
	EngineShowFlags.SetCompositeEditorPrimitives(true);

	bShowWidget = true;

	if (CurrentPreviewScene && CurrentPreviewScene->GetWorld())
	{
		CurrentPreviewScene->GetWorld()->SetBegunPlay(false);
	}

	if (GUnrealEd)
	{
		CurrentPreviewScene->SetSkyCubemap(GUnrealEd->GetThumbnailManager()->AmbientCubemap);
	}

	SetUseLevelWorld(bUseLevelWorld);

	
}

FDialogBuilderViewportClient::~FDialogBuilderViewportClient()
{
	if (GEditor)
	{
		GEditor->SelectNone(true, true, false);
	
		GEditor->NoteSelectionChange();
	}
	// Ensure that an in-progress transaction is ended
	EndTransaction();
}

void FDialogBuilderViewportClient::SetUseLevelWorld(bool bInUseLevelWorld)
{
	bUseLevelWorld = bInUseLevelWorld;	
	if (bInUseLevelWorld)
	{
		//fetch renderer settings
		const URendererSettings* RendererSettings = GetDefault<URendererSettings>();
		const bool bLumenGIEnabled = RendererSettings->DynamicGlobalIllumination == EDynamicGlobalIlluminationMethod::Lumen;
		const bool bLumenReflectionsEnabled = RendererSettings->Reflections == EReflectionMethod::Lumen;
		const bool bMegaLightsEnabled = RendererSettings->bEnableMegaLights;
		
		{
			EngineShowFlags.EnableAdvancedFeatures();
			EngineShowFlags.SetLumenGlobalIllumination(bLumenGIEnabled);
			EngineShowFlags.SetLumenReflections(bLumenReflectionsEnabled);
			EngineShowFlags.SetMegaLights(bMegaLightsEnabled);
		}

		PreviewScene = nullptr;
	}
	else
	{
		EngineShowFlags.DisableAdvancedFeatures();
		PreviewScene = CurrentPreviewScene;
	}

	if (GEditor)
	{
		GEditor->SelectNone(true, true, false);
	}
}

void FDialogBuilderViewportClient::BeginTransaction(const FText& Description)
{
	if (!ScopedTransaction)
	{
		if (!ScopedTransaction)
		{
			ScopedTransaction = new FScopedTransaction(Description);
		}
	}

}

void FDialogBuilderViewportClient::EndTransaction()
{
	if (ScopedTransaction)
	{
		delete ScopedTransaction;
		ScopedTransaction = nullptr;
	}
}


void FDialogBuilderViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);

	

	// Tick the preview scene world.
	if (!GIntraFrameDebuggingGameThread && !bUseLevelWorld)
	{

		// Allow full tick only if preview simulation is enabled and we're not currently in an active SIE or PIE session
		if (GEditor->PlayWorld == NULL && !GEditor->bIsSimulatingInEditor)
		{
			PreviewScene->GetWorld()->Tick(IsRealtime() ? LEVELTICK_All : LEVELTICK_TimeOnly, DeltaSeconds);
		}
		else
		{
			PreviewScene->GetWorld()->Tick(IsRealtime() ? LEVELTICK_ViewportsOnly : LEVELTICK_TimeOnly, DeltaSeconds);
		}
	}
}

bool FDialogBuilderViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
	bool bHandled = GUnrealEd->ComponentVisManager.HandleInputKey(this, EventArgs.Viewport, EventArgs.Key, EventArgs.Event);

	if (!bHandled)
	{
		bHandled = FEditorViewportClient::InputKey(EventArgs);
	}

	return bHandled;
}

void FDialogBuilderViewportClient::ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY)
{
	const FViewportClick Click(&View, this, Key, Event, HitX, HitY);
	TSharedPtr<FDialogBuilderEditor> DialogEditor = Editor.Pin();

	const bool bNoteSelectionChange = true;
	const bool bDeselectBSPSurfs = true;
	const bool bWarnAboutManyActors = false;


	if (!DialogEditor.IsValid()) return;

	if (!HitProxy)
	{
		DialogEditor->SelectDialogDefinition(nullptr);
		return;
	}
	if (HitProxy->IsA(HWidgetAxis::StaticGetType()))
	{
		const bool bOldModeWidgets1 = EngineShowFlags.ModeWidgets;
		const bool bOldModeWidgets2 = View.Family->EngineShowFlags.ModeWidgets;

		EngineShowFlags.SetModeWidgets(false);
		FSceneViewFamily* SceneViewFamily = const_cast<FSceneViewFamily*>(View.Family);
		SceneViewFamily->EngineShowFlags.SetModeWidgets(false);
		bool bWasWidgetDragging = Widget->IsDragging();
		Widget->SetDragging(false);

		// Invalidate the hit proxy map so it will be rendered out again when GetHitProxy
		// is called
		Viewport->InvalidateHitProxy();

		// This will actually re-render the viewport's hit proxies!
		HHitProxy* HitProxyWithoutAxisWidgets = Viewport->GetHitProxy(HitX, HitY);
		if (HitProxyWithoutAxisWidgets != NULL && !HitProxyWithoutAxisWidgets->IsA(HWidgetAxis::StaticGetType()))
		{
			// Try this again, but without the widget this time!
			ProcessClick(View, HitProxyWithoutAxisWidgets, Key, Event, HitX, HitY);
		}

		// Undo the evil
		EngineShowFlags.SetModeWidgets(bOldModeWidgets1);
		SceneViewFamily->EngineShowFlags.SetModeWidgets(bOldModeWidgets2);

		Widget->SetDragging(bWasWidgetDragging);

		// Invalidate the hit proxy map again so that it'll be refreshed with the original
		// scene contents if we need it again later.
		Viewport->InvalidateHitProxy();
		return;
	}

	if (GUnrealEd->ComponentVisManager.HandleClick(this, HitProxy, Click))
	{
		// Component Vis Manager handled this click, no need to do anything
		return;
	}


	UObject* ObjectForDetails = nullptr;
	AActor* ActorToSelect = nullptr;

	if (HitProxy->IsA(HActor::StaticGetType()))
	{
		if (!CanSelectActorFromHitProxy(HitProxy)) return;

		HActor* ActorProxy = (HActor*)HitProxy;
		if (ActorProxy && ActorProxy->Actor && ActorProxy->PrimComponent)
		{
			ActorToSelect = ActorProxy->Actor;
			ObjectForDetails = ActorToSelect;
		}

		UDialogDefinition* DialogDefinition = DialogEditor->FindDialogDefinitionByActor(ActorToSelect);

		if (DialogDefinition)
		{
			DialogEditor->SelectDialogDefinition(DialogDefinition);
		}
		else
		{
			DialogEditor->SetDetailsObject(ObjectForDetails);

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


		


		Invalidate();
	}
}

void FDialogBuilderViewportClient::MouseMove(FViewport* InViewport, int32 InX, int32 InY)
{
	HHitProxy* const HitResult = InViewport ? InViewport->GetHitProxy(InX, InY) : nullptr;
	UpdateHoverFromHitProxy(HitResult);

	return FEditorViewportClient::MouseMove(InViewport, InX, InY);
}

EMouseCursor::Type FDialogBuilderViewportClient::GetCursor(FViewport* InViewport, int32 InX, int32 InY)
{
	if (MouseCursor.IsSet())
	{
		return MouseCursor.GetValue();
	}

	return FEditorViewportClient::GetCursor(InViewport, InX, InY);
}


void FDialogBuilderViewportClient::TrackingStarted(const struct FInputEventState& InInputState, bool bIsDraggingWidget, bool bNudge)
{
	if (!bIsManipulating && bIsDraggingWidget)
	{
		HandleBeginTransform();
	}
}

void FDialogBuilderViewportClient::TrackingStopped()
{
	if (bIsManipulating)
	{
		HandleEndTransform();
	}
}

bool FDialogBuilderViewportClient::BeginTransform(const FGizmoState& InState)
{
	return HandleBeginTransform();
}

bool FDialogBuilderViewportClient::EndTransform(const FGizmoState& InState)
{
	return HandleEndTransform();
}

bool FDialogBuilderViewportClient::HandleBeginTransform()
{
	if (!bIsManipulating)
	{
		// Suspend component modification during each delta step to avoid recording unnecessary overhead into the transaction buffer
		GEditor->DisableDeltaModification(true);

		// Begin transaction
		BeginTransaction(NSLOCTEXT("UnrealEd", "ModifyComponents", "Modify Component(s)"));
		bIsManipulating = true;
		return true;
	}

	return false;
}

bool FDialogBuilderViewportClient::HandleEndTransform()
{
	if (bIsManipulating)
	{
		bIsManipulating = false;
		EndTransaction();

		// Restore component delta modification
		GEditor->DisableDeltaModification(false);

		return true;
	}

	return false;
}

bool FDialogBuilderViewportClient::InputWidgetDelta(FViewport* InViewport, EAxisList::Type CurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale)
{
	if (!bIsManipulating || CurrentAxis == EAxisList::None || !GEditor)
	{
		return false;
	}

	if (GUnrealEd->ComponentVisManager.HandleInputDelta(this, InViewport, Drag, Rot, Scale))
	{
		GUnrealEd->RedrawLevelEditingViewports();
		Invalidate();
		return true;
	}

	USelection* SelectedActors = GEditor->GetSelectedActors();
	if (!SelectedActors || SelectedActors->Num() == 0)
	{
		return false;
	}

	FVector ModifiedScale = Scale;
	if (!Scale.IsNearlyZero())
	{
		FSnappingUtils::AdjustPercentageBasedScale(ModifiedScale);
	}
	else
	{
		ModifiedScale = FVector::ZeroVector;
	}

	const bool bAltDown = InViewport && (InViewport->KeyState(EKeys::LeftAlt) || InViewport->KeyState(EKeys::RightAlt));
	const bool bShiftDown = InViewport && (InViewport->KeyState(EKeys::LeftShift) || InViewport->KeyState(EKeys::RightShift));
	const bool bControlDown = InViewport && (InViewport->KeyState(EKeys::LeftControl) || InViewport->KeyState(EKeys::RightControl));

	bool bHandled = false;

	for (FSelectionIterator It(*SelectedActors); It; ++It)
	{
		AActor* SelectedActor = Cast<AActor>(*It);
		if (!IsValid(SelectedActor))
		{
			continue;
		}

		SelectedActor->Modify();

		GEditor->ApplyDeltaToActor(
			SelectedActor,
			true,
			&Drag,
			&Rot,
			&ModifiedScale,
			bAltDown,
			bShiftDown,
			bControlDown);

		bHandled = true;
	}

	if (bHandled)
	{
		GUnrealEd->RedrawLevelEditingViewports();
		Invalidate();
	}

	return bHandled;
}

FVector FDialogBuilderViewportClient::GetWidgetLocation() const
{
	FVector ComponentVisWidgetLocation;
	if (GUnrealEd->ComponentVisManager.IsVisualizingArchetype() &&
		GUnrealEd->ComponentVisManager.GetWidgetLocation(this, ComponentVisWidgetLocation))
	{
		return ComponentVisWidgetLocation;
	}

	USelection* SelectedActors = GEditor ? GEditor->GetSelectedActors() : nullptr;
	if (SelectedActors && SelectedActors->Num() > 0)
	{
		if (AActor* SelectedActor = Cast<AActor>(SelectedActors->GetTop(AActor::StaticClass())))
		{
			return SelectedActor->GetActorLocation();
		}
	}

	return FVector::ZeroVector;
}

FMatrix FDialogBuilderViewportClient::GetWidgetCoordSystem() const
{
	FMatrix ComponentVisWidgetCoordSystem;
	if (GUnrealEd->ComponentVisManager.IsVisualizingArchetype() &&
		GUnrealEd->ComponentVisManager.GetCustomInputCoordinateSystem(this, ComponentVisWidgetCoordSystem))
	{
		return ComponentVisWidgetCoordSystem;
	}

	FMatrix Matrix = FMatrix::Identity;

	if (GetWidgetCoordSystemSpace() == COORD_Local)
	{
		USelection* SelectedActors = GEditor ? GEditor->GetSelectedActors() : nullptr;
		if (SelectedActors && SelectedActors->Num() > 0)
		{
			if (AActor* SelectedActor = Cast<AActor>(SelectedActors->GetTop(AActor::StaticClass())))
			{
				Matrix = FQuatRotationMatrix(SelectedActor->GetActorQuat());
			}
		}
	}

	if (!Matrix.Equals(FMatrix::Identity))
	{
		Matrix.RemoveScaling();
	}

	return Matrix;
}


UE::Widget::EWidgetMode FDialogBuilderViewportClient::GetWidgetMode() const
{
	if (GUnrealEd->ComponentVisManager.IsActive() &&
		GUnrealEd->ComponentVisManager.IsVisualizingArchetype())
	{
		return WidgetMode;
	}

	USelection* SelectedActors = GEditor ? GEditor->GetSelectedActors() : nullptr;
	if (!SelectedActors || SelectedActors->Num() == 0)
	{
		return UE::Widget::WM_None;
	}

	for (FSelectionIterator It(*SelectedActors); It; ++It)
	{
		AActor* SelectedActor = Cast<AActor>(*It);
		if (IsValid(SelectedActor))
		{
			return WidgetMode;
		}
	}

	return UE::Widget::WM_None;
}

ECoordSystem FDialogBuilderViewportClient::GetWidgetCoordSystemSpace() const
{
	return WidgetCoordSystem;
}


void FDialogBuilderViewportClient::SetWidgetMode(UE::Widget::EWidgetMode NewMode)
{
	WidgetMode = NewMode;
}

void FDialogBuilderViewportClient::SetWidgetCoordSystemSpace(ECoordSystem NewCoordSystem)
{
	WidgetCoordSystem = NewCoordSystem;
}


bool FDialogBuilderViewportClient::GetShowGrid()
{
	return GetDefault<UDialogBuilderSetting>()->bShowGrid;
}

void FDialogBuilderViewportClient::ToggleShowGrid()
{
	UDialogBuilderSetting* Settings = GetMutableDefault<UDialogBuilderSetting>();

	bool bShowGrid = Settings->bShowGrid;
	bShowGrid = !bShowGrid;

	DrawHelper.bDrawGrid = bShowGrid;

	Settings->bShowGrid = bShowGrid;
	Settings->PostEditChange();

	Invalidate();
}


void FDialogBuilderViewportClient::UpdateHoverFromHitProxy(HHitProxy* const InHitProxy)
{
	MouseCursor.Reset();
}

bool FDialogBuilderViewportClient::CanSelectActorFromHitProxy(HHitProxy* const InHitProxy)
{
	const TSharedPtr<FDialogBuilderEditor> DialogEditor = Editor.Pin();
	if (!DialogEditor.IsValid()) return false;

	if (InHitProxy == nullptr) return false;

	const HActor* ActorProxy = HitProxyCast<HActor>(InHitProxy);
	if (ActorProxy == nullptr || !IsValid(ActorProxy->Actor))
	{
		return false;
	}

	const AActor* const HitActor = ActorProxy->Actor;

	if (DialogEditor->GetSequencePivot() == HitActor)
	{
		return true;
	}

	// Replace `GetParticipantTrackActors()` with your actual editor accessor.
	for (AActor* ParticipantActor : DialogEditor->GetDialogDefinitionActors())
	{
		if (ParticipantActor == HitActor)
		{
			return true;
		}
	}

	return false;
}


void FDialogBuilderViewportClient::InvalidatePreview(bool bResetCamera)
{
	TSharedPtr<FDialogBuilderEditor> DialogEditor = Editor.Pin();

	if (!DialogEditor.IsValid()) return;
	
	DialogEditor->UpdateDialogDefinitions();

	Invalidate();

	if (bResetCamera)
	{
		ResetCamera();
	}
}

void FDialogBuilderViewportClient::ResetCamera()
{
	TSharedPtr<FDialogBuilderEditor> DialogEditor = Editor.Pin();
	if (!DialogEditor.IsValid())
	{
		return;
	}

	AActor* SequencePivot = DialogEditor->GetSequencePivot();

	FVector FocusLocation = FVector::ZeroVector;
	FBox CombinedBounds(ForceInit);

	for (AActor* DialogActor : DialogEditor->GetDialogDefinitionActors())
	{
		if (IsValid(DialogActor))
		{
			CombinedBounds += DialogActor->GetComponentsBoundingBox(true);
		}
	}

	if (IsValid(SequencePivot))
	{
		FocusLocation = SequencePivot->GetActorLocation();
	}
	else if (CombinedBounds.IsValid)
	{
		FocusLocation = CombinedBounds.GetCenter();
	}

	USceneThumbnailInfo* ThumbnailInfo = USceneThumbnailInfo::StaticClass()->GetDefaultObject<USceneThumbnailInfo>();

	const float BoundsRadius = CombinedBounds.IsValid ? CombinedBounds.GetExtent().Length() : 256.0f;
	double OrbitZoom = ThumbnailInfo ? ThumbnailInfo->OrbitZoom : 0.0;

	if (BoundsRadius + OrbitZoom < 0.0)
	{
		OrbitZoom = -BoundsRadius;
	}

	const double TargetDistance = BoundsRadius > 0.0f ? BoundsRadius : 256.0f;
	const FRotator ThumbnailAngle = ThumbnailInfo
		? FRotator(ThumbnailInfo->OrbitPitch, ThumbnailInfo->OrbitYaw, 0.0f)
		: FRotator(-15.0f, -135.0f, 0.0f);

	ToggleOrbitCamera(true);
	SetViewLocationForOrbiting(FocusLocation);
	SetViewLocation(GetViewLocation() + FVector(0.0f, TargetDistance * 1.5f + OrbitZoom - 256.0f, 0.0f));
	SetViewRotation(ThumbnailAngle);

	Invalidate();
}

void FDialogBuilderViewportClient::FocusViewportToSelection()
{
	USelection* SelectedActors = GEditor ? GEditor->GetSelectedActors() : nullptr;

	FBox BoundingBox(ForceInit);

	for (FSelectionIterator It(*SelectedActors); It; ++It)
	{
		if (AActor* SelectedActor = Cast<AActor>(*It))
		{
			BoundingBox += SelectedActor->GetRootComponent()->Bounds.GetBox();
		}
	}

	if (BoundingBox.IsValid)
	{
		FocusViewportOnBox(BoundingBox);
	}
}

#undef LOCTEXT_NAMESPACE