#include "SDialogPreviewViewport.h"

#include "DialogBuilderEditor.h"
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
#include "EditorModeManager.h"
#include "AdvancedPreviewScene.h"
#include "EditorModeManager.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "AdvancedPreviewSceneMenus.h"
#include "ViewportToolbar/UnrealEdViewportToolbar.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

class FDialogEditorViewportClient final : public FEditorViewportClient
{
public:
	FDialogEditorViewportClient(
		const TSharedRef<FAdvancedPreviewScene>& InPreviewScene,
		const TSharedRef<SEditorViewport>& InViewport,
		const TSharedRef<FDialogBuilderEditor>& InEditor)
		: FEditorViewportClient(nullptr, &InPreviewScene.Get(), InViewport)
		, Editor(InEditor)
	{
		ViewportType = LVT_Perspective;
		SetViewLocation(FVector(-250.0, 0.0, 125.0));
		SetViewRotation(FRotator(0.0, 0.0, 0.0));
		SetRealtime(true);
		bSetListenerPosition = false;

		EngineShowFlags.SetSelectionOutline(true);
		EngineShowFlags.SetCompositeEditorPrimitives(true);

		bShowWidget = true;
		SetWidgetMode(UE::Widget::WM_Translate);
	}


	void SetUseLevelWorld(bool bUseLevelWorld)
	{
		if (bUseLevelWorld)
		{
			UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
			//SetWorld(World);
		}
		else
		{
			// When not using the level, keep rendering the preview scene world.
			//SetWorld(nullptr);
		}
	}
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override
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


	// --- Gizmo (transform widget) support ---

	virtual UE::Widget::EWidgetMode GetWidgetMode() const override
	{
		// Disable translate/rotate/scale when nothing is selected.
		return HasValidSelection() ? WidgetMode : UE::Widget::WM_None;
	}

	virtual FVector GetWidgetLocation() const override
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

	virtual ECoordSystem GetWidgetCoordSystemSpace() const override
	{
		// Local vs World. Start with world; you can add a toggle later.
		return COORD_World;
	}

	virtual bool InputWidgetDelta(FViewport* InViewport, EAxisList::Type CurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale) override
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

		// Apply deltas (minimal). Add transactions if you want undo/redo.
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

	// Helper for switching mode (optional)
	void SetWidgetMode(UE::Widget::EWidgetMode InMode)
	{
		WidgetMode = InMode;
	}

private:
	bool HasValidSelection() const
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
	void HandleClick(FViewport* InViewport)
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

		// Prefer showing/selecting the root primitive component if we only got an actor.
		if (ActorToSelect && !ComponentToSelect)
		{
			if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(ActorToSelect->GetRootComponent()))
			{
				ComponentToSelect = PrimComp;
				ObjectForDetails = PrimComp;
			}
		}

		// Update details panel in your toolkit
		Editor->SetDetailsObject(ObjectForDetails);

		// ------- Selection (drives outline) -------
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

private:
	TSharedRef<FDialogBuilderEditor> Editor;
	UE::Widget::EWidgetMode WidgetMode = UE::Widget::WM_Translate;
};

//------------------ SDialogPreviewViewport ------------------

void SDialogPreviewViewport::Construct(const FArguments& InArgs, TSharedRef<FDialogBuilderEditor> InEditor, TSharedRef<FAdvancedPreviewScene> InPreviewScene)
{
	AssetEditorToolkitPtr = InEditor;
	Editor = InEditor;
	PreviewScene = InPreviewScene;

	SEditorViewport::Construct(SEditorViewport::FArguments());
}


TSharedRef<FEditorViewportClient> SDialogPreviewViewport::MakeEditorViewportClient()
{
	TSharedRef<FDialogEditorViewportClient> VClient =
		MakeShared<FDialogEditorViewportClient>(PreviewScene.ToSharedRef(), SharedThis(this), Editor.Pin().ToSharedRef());
	ViewportClient = VClient;
	VClient->SetUseLevelWorld(bUseLevelWorld);
	return VClient;
}

void SDialogPreviewViewport::SetUseLevelWorld(bool bInUseLevelWorld)
{
	bUseLevelWorld = bInUseLevelWorld;

	if (ViewportClient.IsValid())
	{
		static_cast<FDialogEditorViewportClient*>(ViewportClient.Get())->SetUseLevelWorld(bUseLevelWorld);
		ViewportClient->Invalidate();
	}
}

TSharedPtr<SWidget> SDialogPreviewViewport::BuildViewportToolbar()
{
	const FName ViewportToolbarName = "DialogSequencer.ViewportToolbar";

	if (!UToolMenus::Get()->IsMenuRegistered(ViewportToolbarName))
	{
		UToolMenu* const ViewportToolbarMenu = UToolMenus::Get()->RegisterMenu(
			ViewportToolbarName, NAME_None /* parent */, EMultiBoxType::SlimHorizontalToolBar
		);

		ViewportToolbarMenu->StyleName = "ViewportToolbar";

		// Add the left-aligned part of the viewport toolbar.
		{
			FToolMenuSection& LeftSection = ViewportToolbarMenu->AddSection("Left");

			LeftSection.AddEntry(UE::UnrealEd::CreateTransformsSubmenu());
			// TODO: Needs specific Select menu for Skel Mesh.
			LeftSection.AddEntry(UE::UnrealEd::CreateSnappingSubmenu());
		}

		// Add the right-aligned part of the viewport toolbar.
		{
			FToolMenuSection& RightSection = ViewportToolbarMenu->AddSection("Right");
			RightSection.Alignment = EToolMenuSectionAlign::Last;

			// Add the "Camera" submenu.
			{
				// Build the menu name our Camera menu will be using so we can extend it.
				RightSection.AddEntry(UE::UnrealEd::CreateCameraSubmenu(UE::UnrealEd::FViewportCameraMenuOptions().ShowAll()));
				const FName SubmenuName = UToolMenus::JoinMenuPaths(ViewportToolbarName, "Camera");
				//UE::AnimationEditor::ExtendCameraMenu(SubmenuName);
			}

			// Add the "View Modes" sub menu.
			{
				const FName ViewModesMenuName = "DialogSequencer.ViewportToolbar.ViewModes";

				// Stay backward-compatible with the old viewport toolbar.
				{
					const FName ParentSubmenuName = "UnrealEd.ViewportToolbar.View";
					// Create our parent menu.
					if (!UToolMenus::Get()->IsMenuRegistered(ParentSubmenuName))
					{
						UToolMenus::Get()->RegisterMenu(ParentSubmenuName);
					}

					// Register our ToolMenu here first, before we create the submenu, so we can set our parent.
					UToolMenus::Get()->RegisterMenu(ViewModesMenuName, ParentSubmenuName);
				}

				//UE::AnimationEditor::ExtendViewModesSubmenu(ViewModesMenuName);

				RightSection.AddEntry(UE::UnrealEd::CreateViewModesSubmenu());
			}

			/*RightSection.AddEntry(UE::AnimationEditor::CreateShowSubmenu());
			RightSection.AddEntry(UE::AnimationEditor::CreateLODSubmenu());
			RightSection.AddEntry(UE::AnimationEditor::CreateSkinWeightProfileMenu());*/

			// Add Preview Scene Submenu
			{
				const FName AssetViewerProfileMenuName = "AnimationEditor.ViewportToolbar.AssetViewerProfile";
				RightSection.AddEntry(UE::UnrealEd::CreateAssetViewerProfileSubmenu());
				UE::AdvancedPreviewScene::Menus::ExtendAdvancedPreviewSceneSettings(AssetViewerProfileMenuName);
				//UE::AnimationEditor::ExtendPreviewSceneSettingsSubmenu(AssetViewerProfileMenuName);
				UE::UnrealEd::ExtendPreviewSceneSettingsWithTabEntry(AssetViewerProfileMenuName);
			}

			
		}
	}

	FToolMenuContext ViewportToolbarContext;
	{
		/*if (TSharedPtr<IPersonaPreviewScene> PreviewScene = PreviewScenePtr.Pin())
		{
			ViewportToolbarContext.AppendCommandList(PreviewScene->GetCommandList());
		}

		if (TSharedPtr<SAnimationEditorViewportTabBody> TabBody = TabBodyPtr.Pin())
		{
			ViewportToolbarContext.AppendCommandList(TabBody->GetCommandList());
		}*/

		ViewportToolbarContext.AppendCommandList(GetCommandList());

		// Add extenders
		{
			//Extenders.Add(UE::AnimationEditor::GetViewModesLegacyExtenders(TabBodyPtr));
			//ViewportToolbarContext.AddExtender(FExtender::Combine(Extenders));
		}

		// Add the UnrealEd viewport toolbar context.
		{
			UUnrealEdViewportToolbarContext* const ContextObject =
				UE::UnrealEd::CreateViewportToolbarDefaultContext(SharedThis(this));

			ContextObject->AssetEditorToolkit = AssetEditorToolkitPtr;
			ContextObject->PreviewSettingsTabId = FName("AdvancedPreviewTab");// FPersonaTabs::AdvancedPreviewSceneSettingsID;

			ViewportToolbarContext.AddObject(ContextObject);
		}

		// Add the Anim viewport toolbar context.
		{
			//UAnimViewportContext* const ContextObject = NewObject<UAnimViewportContext>();
			//ContextObject->ViewportTabBody = TabBodyPtr.Pin();

			//ViewportToolbarContext.AddObject(ContextObject);
		}

		AssetEditorToolkitPtr.Pin()->InitToolMenuContext(ViewportToolbarContext);
		// Give the asset editor a chance to extend the context.
		/*if (TSharedPtr<SAnimationEditorViewportTabBody> Tab = TabBodyPtr.Pin())
		{
			Tab->GetAssetEditorToolkit()->InitToolMenuContext(ViewportToolbarContext);
		}*/
	}

	return UToolMenus::Get()->GenerateWidget(ViewportToolbarName, ViewportToolbarContext);
	/*TSharedPtr<FDialogBuilderEditor> EditorPinned = Editor.Pin();
	if (!EditorPinned.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	FToolBarBuilder ToolbarBuilder(TSharedPtr<FUICommandList>(), FMultiBoxCustomization::None);

	ToolbarBuilder.AddToolBarButton(
		FUIAction(
			FExecuteAction::CreateSP(EditorPinned.Get(), &FDialogBuilderEditor::SetViewportWorldMode, EDialogViewportWorldMode::PreviewScene),
			FCanExecuteAction(),
			FIsActionChecked::CreateSP(EditorPinned.Get(), &FDialogBuilderEditor::IsViewportWorldMode, EDialogViewportWorldMode::PreviewScene)
		),
		NAME_None,
		FText::FromString(TEXT("Preview Scene")),
		FText::FromString(TEXT("Render the dialog preview/template scene.")),
		FSlateIcon(),
		EUserInterfaceActionType::ToggleButton
	);

	ToolbarBuilder.AddToolBarButton(
		FUIAction(
			FExecuteAction::CreateSP(EditorPinned.Get(), &FDialogBuilderEditor::SetViewportWorldMode, EDialogViewportWorldMode::CurrentLevel),
			FCanExecuteAction(),
			FIsActionChecked::CreateSP(EditorPinned.Get(), &FDialogBuilderEditor::IsViewportWorldMode, EDialogViewportWorldMode::CurrentLevel)
		),
		NAME_None,
		FText::FromString(TEXT("Current Level")),
		FText::FromString(TEXT("Render the current editor level world.")),
		FSlateIcon(),
		EUserInterfaceActionType::ToggleButton
	);

	return ToolbarBuilder.MakeWidget();*/
}