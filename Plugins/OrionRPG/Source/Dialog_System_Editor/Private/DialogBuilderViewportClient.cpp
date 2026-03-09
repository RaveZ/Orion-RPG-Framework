#include "DialogBuilderViewportClient.h"

#include "DialogBuilderEditor.h"
#include "DialogBuilderSetting.h"

#include "AdvancedPreviewScene.h"
#include "Editor.h"
#include "EditorViewportClient.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "SNameComboBox.h"
#include "ToolMenus.h"
#include "Viewports.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Selection.h"
#include "EngineUtils.h"
#include "HitProxies.h"
#include "ComponentVisualizer.h"
#include "Styling/AppStyle.h"
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

void FDialogBuilderViewportClient::SetUseLevelWorld(bool bInUseLevelWorld)
{
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


bool FDialogBuilderViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
	/*const bool bHandled = FEditorViewportClient::InputKey(EventArgs);

	if (EventArgs.Key == EKeys::LeftMouseButton && EventArgs.Event == IE_Pressed)
	{
		HandleClick(EventArgs.Viewport);
		return true;
	}

	if (EventArgs.Key == EKeys::LeftMouseButton && EventArgs.Event == IE_Released)
	{
		EndTransaction();
	}

	bShowWidget = HasValidSelection();
	return bHandled;*/
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

	const bool bNoteSelectionChange = true;
	const bool bDeselectBSPSurfs = true;
	const bool bWarnAboutManyActors = false;

	if (!HitProxy)
	{
		GEditor->SelectNone(bNoteSelectionChange, bDeselectBSPSurfs, bWarnAboutManyActors);
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
		HActor* ActorProxy = (HActor*)HitProxy;
		if (ActorProxy && ActorProxy->Actor && ActorProxy->PrimComponent)
		{

			ActorToSelect = ActorProxy->Actor;
			ObjectForDetails = ActorToSelect;
		}

		if (TSharedPtr<FDialogBuilderEditor> EditorPinned = Editor.Pin())
		{
			EditorPinned->SetDetailsObject(ObjectForDetails);
		}

	if (GEditor)
		{
		
			GEditor->SelectNone(bNoteSelectionChange, bDeselectBSPSurfs, bWarnAboutManyActors);
			if (ActorToSelect)
			{
				GEditor->SelectActor(ActorToSelect, true, bNoteSelectionChange);
			}

			GEditor->NoteSelectionChange();
		}

		Invalidate();
	}
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
	USceneComponent* TargetSceneComp = nullptr;

	if (USelection* SelectedComponents = GEditor ? GEditor->GetSelectedComponents() : nullptr)
	{
		if (UActorComponent* Component = Cast<UActorComponent>(SelectedComponents->GetTop(UActorComponent::StaticClass())))
		{
			TargetSceneComp = Cast<USceneComponent>(Component);
		}
	}

	if (!TargetSceneComp)
	{
		if (USelection* SelectedActors = GEditor ? GEditor->GetSelectedActors() : nullptr)
		{
			if (AActor* Actor = Cast<AActor>(SelectedActors->GetTop(AActor::StaticClass())))
			{
				TargetSceneComp = Actor->GetRootComponent();
			}
		}
	}

	if (!TargetSceneComp)
	{
		return false;
	}

	BeginTransaction(NSLOCTEXT("DialogPreviewViewport", "MovePreviewObject", "Move Preview Object"));

	if (AActor* Owner = TargetSceneComp->GetOwner())
	{
		Owner->Modify();
	}

	TargetSceneComp->Modify();

	if (!Drag.IsNearlyZero())
	{
		TargetSceneComp->AddWorldOffset(Drag, false);
	}

	if (!Rot.IsNearlyZero())
	{
		TargetSceneComp->AddWorldRotation(Rot, false);
	}

	if (!Scale.IsNearlyZero())
	{
		const FVector NewScale = TargetSceneComp->GetComponentScale() + Scale;
		TargetSceneComp->SetWorldScale3D(NewScale);
	}

	return true;
}

UE::Widget::EWidgetMode FDialogBuilderViewportClient::GetWidgetMode() const
{
	return HasValidSelection() ? WidgetMode : UE::Widget::WM_None;
}

FVector FDialogBuilderViewportClient::GetWidgetLocation() const
{
	if (USelection* SelectedComponents = GEditor ? GEditor->GetSelectedComponents() : nullptr)
	{
		if (UActorComponent* Component = Cast<UActorComponent>(SelectedComponents->GetTop(UActorComponent::StaticClass())))
		{
			if (USceneComponent* SceneComp = Cast<USceneComponent>(Component))
			{
				return SceneComp->GetComponentLocation();
			}
		}
	}

	if (USelection* SelectedActors = GEditor ? GEditor->GetSelectedActors() : nullptr)
	{
		if (AActor* Actor = Cast<AActor>(SelectedActors->GetTop(AActor::StaticClass())))
		{
			return Actor->GetActorLocation();
		}
	}

	return FVector::ZeroVector;
}

ECoordSystem FDialogBuilderViewportClient::GetWidgetCoordSystemSpace() const
{
	return COORD_World;
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


bool FDialogBuilderViewportClient::HasValidSelection() const
{
	if (!GEditor)
	{
		return false;
	}

	if (USelection* SelectedComponents = GEditor->GetSelectedComponents())
	{
		if (SelectedComponents->Num() > 0)
		{
			return true;
		}
	}

	if (USelection* SelectedActors = GEditor->GetSelectedActors())
	{
		if (SelectedActors->Num() > 0)
		{
			return true;
		}
	}

	return false;
}


#undef LOCTEXT_NAMESPACE