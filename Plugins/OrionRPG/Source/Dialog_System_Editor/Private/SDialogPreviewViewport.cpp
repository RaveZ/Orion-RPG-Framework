#include "SDialogPreviewViewport.h"

#include "DialogBuilderEditor.h"
#include "DialogBuilderViewportClient.h"
#include "DialogBuilder_EditorCommands.h"
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
#include "EditorModeManager.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "AdvancedPreviewSceneMenus.h"
#include "ViewportToolbar/UnrealEdViewportToolbar.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "DialogPreviewViewport"

void SDialogPreviewViewport::Construct(const FArguments& InArgs, TSharedRef<FDialogBuilderEditor> InEditor, TSharedRef<FPreviewScene> InPreviewScene)
{
	AssetEditorToolkitPtr = InEditor;
	Editor = InEditor;
	PreviewScene = InPreviewScene;

	SEditorViewport::Construct(SEditorViewport::FArguments());
}


TSharedRef<FEditorViewportClient> SDialogPreviewViewport::MakeEditorViewportClient()
{
	ViewportClient = MakeShared<FDialogBuilderViewportClient>(PreviewScene.ToSharedRef(), SharedThis(this), Editor.Pin().ToSharedRef());
	ViewportClient->SetRealtime(true);
	ViewportClient->bSetListenerPosition = false;

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
		const TSharedPtr<FDialogBuilderViewportClient> DialogClient =
			StaticCastSharedPtr<FDialogBuilderViewportClient>(ViewportClient);

		if (DialogClient.IsValid())
		{
			DialogClient->SetUseLevelWorld(bInUseLevelWorld);
		}
	}



}

void SDialogPreviewViewport::BindCommands()
{
	//CommandList->Append(Editor->GetToolkitCommands());
	SEditorViewport::BindCommands();

	const FDialogBuilder_EditorCommands& Commands = FDialogBuilder_EditorCommands::Get();
	CommandList->MapAction(
		Commands.ShowGrid,
		FExecuteAction::CreateSP(ViewportClient.Get(), &FDialogBuilderViewportClient::ToggleShowGrid),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(ViewportClient.Get(), &FDialogBuilderViewportClient::GetShowGrid));

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

			const FName AssetViewerProfileMenuName("DialogEditor.ViewportToolbar.AssetViewerProfile");
			UE::AdvancedPreviewScene::Menus::ExtendAdvancedPreviewSceneSettings(AssetViewerProfileMenuName);
			UE::UnrealEd::ExtendPreviewSceneSettingsWithTabEntry(AssetViewerProfileMenuName);
			
			RightSection.AddEntry(UE::UnrealEd::CreateShowSubmenu(FNewToolMenuDelegate::CreateLambda(
				[](UToolMenu* Submenu) -> void
				{
					FToolMenuSection& UnnamedSection = Submenu->FindOrAddSection(NAME_None);
					UnnamedSection.AddMenuEntry(FDialogBuilder_EditorCommands::Get().ShowGrid);
				})
			));
			
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