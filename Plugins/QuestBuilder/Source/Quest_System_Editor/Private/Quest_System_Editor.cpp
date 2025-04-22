// Copyright 2024 Ivan Chandra. All Rights Reserved.

#include "Quest_System_Editor.h"
#include "Quest_System_EditorStyle.h"
#include "Quest_System_EditorCommands.h"
#include "QuestSystemGraphNode.h"
#include "Task/QuestTask.h"
#include "IAssetTools.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "GraphNodeFactory.h"
#include "Styling/SlateStyleRegistry.h"
#include <IAssetTools.h>


#define LOCTEXT_NAMESPACE "FQuest_System_EditorModule"

static const FName Quest_System_EditorTabName("Quest_System_Editor");
TSharedPtr<FSlateStyleSet> FQuest_System_EditorModule::CustomAssetsEditorSlateStyle;

void FQuest_System_EditorModule::StartupModule()
{
	FQuest_System_EditorStyle::Initialize();
	FQuest_System_EditorStyle::ReloadTextures();

	FQuest_System_EditorCommands::Register();
	
	//Register GraphNodeFactory
	GraphPanelNodeFactory_QuestEditor = MakeShareable(new FQuestSystemNodeFactory());
	FEdGraphUtilities::RegisterVisualNodeFactory(GraphPanelNodeFactory_QuestEditor);
	

	// Register the slate styles	
	const FString ProjectResourceDir = FPaths::ProjectPluginsDir() / TEXT("QuestBuilder/Resources");
	CustomAssetsEditorSlateStyle = MakeShareable(new FSlateStyleSet("QuestEditor"));
	CustomAssetsEditorSlateStyle->Set("CustomAsset.Thumbnail", new FSlateImageBrush(FString(ProjectResourceDir / "Icon128.png"), FVector2D(128.0f, 128.0f)));
	CustomAssetsEditorSlateStyle->Set("ClassIcon.CustomActor", new FSlateImageBrush(FString(ProjectResourceDir / "Icon128.png"), FVector2D(32.0f, 32.0f)));
	CustomAssetsEditorSlateStyle->Set("ClassIcon.UAssetDefinition_CustomObject", new FSlateImageBrush(FString(ProjectResourceDir / "Icon128.png"), FVector2D(32.0f, 32.0f)));

	FSlateStyleRegistry::RegisterSlateStyle(*CustomAssetsEditorSlateStyle.Get());
}

void FQuest_System_EditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	if (GraphPanelNodeFactory_QuestEditor.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(GraphPanelNodeFactory_QuestEditor);
		GraphPanelNodeFactory_QuestEditor.Reset();
	}

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FQuest_System_EditorStyle::Shutdown();

	FQuest_System_EditorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(Quest_System_EditorTabName);

	ClassCache.Reset();
}

TSharedRef<SDockTab> FQuest_System_EditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
		//Load Property Module
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		//FDetailsViewArgs is a struct of settings to customize our Details View Widget
		FDetailsViewArgs Args;
		Args.bHideSelectionTip = true;

	// Return the property widget as the content of the tab
		FText WidgetText = FText::Format(
			LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
			FText::FromString(TEXT("FQuest_System_EditorModule::OnSpawnPluginTab")),
			FText::FromString(TEXT("Quest_System_Editor.cpp"))
			);

		return SNew(SDockTab)
			.TabRole(ETabRole::NomadTab)
			[
				// Put your tab content here!
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(WidgetText)
				]
			];
}

void FQuest_System_EditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(Quest_System_EditorTabName);
}

void FQuest_System_EditorModule::CheckClassCache()
{
	if (!ClassCache.IsValid())
	{
		ClassCache = MakeShareable(new FGraphNodeClassHelper(UQuestTask::StaticClass()));
		FGraphNodeClassHelper::AddObservedBlueprintClasses(UQuestTask::StaticClass());
		ClassCache->UpdateAvailableBlueprintClasses();
	}
}

void FQuest_System_EditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FQuest_System_EditorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FQuest_System_EditorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FQuest_System_EditorModule, Quest_System_Editor)

