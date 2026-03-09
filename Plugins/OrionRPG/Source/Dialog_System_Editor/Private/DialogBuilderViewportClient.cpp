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
#include "EditorModeManager.h"
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
	, Editor(InEditor)
	, CurrentPreviewScene(InPreviewScene)
{
	ViewportType = LVT_Perspective;
	SetViewLocation(FVector(-250.0, 0.0, 125.0));
	SetViewRotation(FRotator(0.0, 0.0, 0.0));
	SetRealtime(true);
	bSetListenerPosition = false;


	// Set if the grid will be drawn
	DrawHelper.bDrawGrid = GetDefault<UDialogBuilderSetting>()->bShowGrid;

	EngineShowFlags.SetSelectionOutline(true);
	EngineShowFlags.SetCompositeEditorPrimitives(true);

	bShowWidget = true;
	SetWidgetMode(UE::Widget::WM_Translate);

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

bool FDialogBuilderViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
	const bool bHandled = FEditorViewportClient::InputKey(EventArgs);

	if (EventArgs.Key == EKeys::LeftMouseButton && EventArgs.Event == IE_Pressed)
	{
		HandleClick(EventArgs.Viewport);
		return true;
	}

	bShowWidget = HasValidSelection();
	return bHandled;
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

	const FScopedTransaction Transaction(NSLOCTEXT("DialogPreviewViewport", "MovePreviewObject", "Move Preview Object"));
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

void FDialogBuilderViewportClient::SetWidgetMode(UE::Widget::EWidgetMode InMode)
{
	WidgetMode = InMode;
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

void FDialogBuilderViewportClient::HandleClick(FViewport* InViewport)
{
	if (!InViewport)
	{
		return;
	}

	const int32 MouseX = InViewport->GetMouseX();
	const int32 MouseY = InViewport->GetMouseY();

	HHitProxy* HitProxy = InViewport->GetHitProxy(MouseX, MouseY);

	UObject* ObjectForDetails = nullptr;
	UActorComponent* ComponentToSelect = nullptr;
	AActor* ActorToSelect = nullptr;

	if (HitProxy)
	{
		if (HitProxy->IsA(HActor::StaticGetType()))
		{
			const HActor* ActorProxy = static_cast<HActor*>(HitProxy);
			ActorToSelect = ActorProxy->Actor;
			ObjectForDetails = ActorToSelect;
		}
		else if (HitProxy->IsA(HComponentVisProxy::StaticGetType()))
		{
			const HComponentVisProxy* ComponentProxy = static_cast<HComponentVisProxy*>(HitProxy);

			const UActorComponent* HitComponentConst = ComponentProxy->Component.Get();
			ComponentToSelect = const_cast<UActorComponent*>(HitComponentConst);

			ObjectForDetails = ComponentToSelect;
			ActorToSelect = ComponentToSelect ? ComponentToSelect->GetOwner() : nullptr;
		}
	}

	if (ActorToSelect && !ComponentToSelect)
	{
		if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(ActorToSelect->GetRootComponent()))
		{
			ComponentToSelect = PrimComp;
			ObjectForDetails = PrimComp;
		}
	}

	if (TSharedPtr<FDialogBuilderEditor> EditorPinned = Editor.Pin())
	{
		EditorPinned->SetDetailsObject(ObjectForDetails);
	}

	if (GEditor)
	{
		const bool bNoteSelectionChange = true;
		const bool bDeselectBSPSurfs = true;
		const bool bWarnAboutManyActors = false;

		GEditor->SelectNone(bNoteSelectionChange, bDeselectBSPSurfs, bWarnAboutManyActors);

		if (ComponentToSelect)
		{
			GEditor->SelectComponent(ComponentToSelect, true, bNoteSelectionChange);
		}
		else if (ActorToSelect)
		{
			GEditor->SelectActor(ActorToSelect, true, bNoteSelectionChange);
		}

		GEditor->NoteSelectionChange();
	}

	Invalidate();
}

#undef LOCTEXT_NAMESPACE