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
#include "Styling/AppStyle.h"
#include "EditorModeManager.h"
#include "AdvancedPreviewScene.h"
#include "EditorModeManager.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "AdvancedPreviewSceneMenus.h"
#include "ViewportToolbar/UnrealEdViewportToolbar.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "DialogPreviewViewport"

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
		CurrentPreviewScene = &InPreviewScene.Get();
	}



	void SetUseLevelWorld(bool bInUseLevelWorld)
	{
		if (bInUseLevelWorld)
		{
			PreviewScene = NULL;
		}
		else
		{
			PreviewScene = CurrentPreviewScene;
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
		if (TSharedPtr<FDialogBuilderEditor> EditorPinned = Editor.Pin())
		{
			EditorPinned->SetDetailsObject(ObjectForDetails);
		}

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
	TWeakPtr<class FDialogBuilderEditor> Editor;
	UE::Widget::EWidgetMode WidgetMode = UE::Widget::WM_Translate;
	FPreviewScene* CurrentPreviewScene;
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
	ViewportClient = StaticCastSharedRef<FEditorViewportClient>(MakeShared<FDialogEditorViewportClient>(PreviewScene.ToSharedRef(), SharedThis(this), Editor.Pin().ToSharedRef()));

	return ViewportClient.ToSharedRef();
}

void SDialogPreviewViewport::SetUseLevelWorld(bool bInUseLevelWorld)
{
	if (bUseLevelWorld == bInUseLevelWorld)
	{
		return;
	}

	bUseLevelWorld = bInUseLevelWorld;

	if (ViewportClient.IsValid())
	{
		const TSharedPtr<FDialogEditorViewportClient> DialogClient =
			StaticCastSharedPtr<FDialogEditorViewportClient>(ViewportClient);

		if (DialogClient.IsValid())
		{
			DialogClient->SetUseLevelWorld(bInUseLevelWorld);
		}
	}



}

TSharedPtr<SWidget> SDialogPreviewViewport::BuildViewportToolbar()
{
	const FName ViewportToolbarName("DialogSequencer.ViewportToolbar");

	if (!UToolMenus::Get()->IsMenuRegistered(ViewportToolbarName))
	{
		UToolMenu* const ViewportToolbarMenu = UToolMenus::Get()->RegisterMenu(
			ViewportToolbarName,
			NAME_None,
			EMultiBoxType::SlimHorizontalToolBar
		);

		ViewportToolbarMenu->StyleName = "ViewportToolbar";

		// Left section (standard-ish viewport actions)
		{
			FToolMenuSection& LeftSection = ViewportToolbarMenu->AddSection("Left");
			LeftSection.AddEntry(UE::UnrealEd::CreateTransformsSubmenu());
			LeftSection.AddEntry(UE::UnrealEd::CreateSnappingSubmenu());
		}

		// Right section (standard menus)
		{
			FToolMenuSection& RightSection = ViewportToolbarMenu->AddSection("Right");
			RightSection.Alignment = EToolMenuSectionAlign::Last;

			RightSection.AddEntry(UE::UnrealEd::CreateCameraSubmenu(UE::UnrealEd::FViewportCameraMenuOptions().ShowAll()));
			RightSection.AddEntry(UE::UnrealEd::CreateViewModesSubmenu());

			RightSection.AddEntry(UE::UnrealEd::CreateAssetViewerProfileSubmenu());

			const FName AssetViewerProfileMenuName("AnimationEditor.ViewportToolbar.AssetViewerProfile");
			UE::AdvancedPreviewScene::Menus::ExtendAdvancedPreviewSceneSettings(AssetViewerProfileMenuName);
			UE::UnrealEd::ExtendPreviewSceneSettingsWithTabEntry(AssetViewerProfileMenuName);
		}
	}

	// Generate Unreal toolbar widget
	TSharedPtr<SWidget> UnrealToolbarWidget;
	{
		FToolMenuContext ViewportToolbarContext;
		ViewportToolbarContext.AppendCommandList(GetCommandList());

		UUnrealEdViewportToolbarContext* const ContextObject =
			UE::UnrealEd::CreateViewportToolbarDefaultContext(SharedThis(this));

		ContextObject->AssetEditorToolkit = AssetEditorToolkitPtr;
		ContextObject->PreviewSettingsTabId = FName("AdvancedPreviewTab");

		ViewportToolbarContext.AddObject(ContextObject);

		if (TSharedPtr<FDialogBuilderEditor> EditorPinned = Editor.Pin())
		{
			EditorPinned->InitToolMenuContext(ViewportToolbarContext);
		}

		UnrealToolbarWidget = UToolMenus::Get()->GenerateWidget(ViewportToolbarName, ViewportToolbarContext);
	}

	// Build your custom right-side widget
	TSharedPtr<SWidget> CustomToolbarWidget = SNullWidget::NullWidget;
	{
		TSharedPtr<FDialogBuilderEditor> EditorPinned = Editor.Pin();
		if (EditorPinned.IsValid())
		{
			FToolBarBuilder ToolbarBuilder(GetCommandList(), FMultiBoxCustomization::None);

			ToolbarBuilder.AddToolBarButton(
				FUIAction(
					FExecuteAction::CreateSP(EditorPinned.Get(), &FDialogBuilderEditor::SetViewportWorldMode, EDialogViewportWorldMode::PreviewScene),
					FCanExecuteAction(),
					FIsActionChecked::CreateSP(EditorPinned.Get(), &FDialogBuilderEditor::IsViewportWorldMode, EDialogViewportWorldMode::PreviewScene)
				),
				NAME_None,
				FText::FromString(TEXT("Preview Scene")),
				FText::FromString(TEXT("Render the dialog preview/template scene.")),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"),
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
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.WorldProperties.Tab"),
				EUserInterfaceActionType::ToggleButton
			);

			CustomToolbarWidget = ToolbarBuilder.MakeWidget();
		}
	}

	// Combine them: Unreal toolbar left, your toolbar right
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			UnrealToolbarWidget.IsValid() ? UnrealToolbarWidget.ToSharedRef() : SNullWidget::NullWidget
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SSpacer)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		[
			CustomToolbarWidget.IsValid() ? CustomToolbarWidget.ToSharedRef() : SNullWidget::NullWidget
		];
}

#undef LOCTEXT_NAMESPACE