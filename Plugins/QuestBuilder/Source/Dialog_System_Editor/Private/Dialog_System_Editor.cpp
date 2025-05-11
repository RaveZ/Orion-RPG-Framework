// Copyright 2024 Ivan Chandra. All Rights Reserved.

#include "Dialog_System_Editor.h"
#include "DialogBuilder_EditorStyle.h"
#include "DialogBuilder_EditorCommands.h"
#include "QuestBuilderNode.h"
#include "Task/QuestTask.h"
#include "IAssetTools.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "ISettingsContainer.h"
#include <ISettingsCategory.h>
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "Styling/SlateStyleRegistry.h"
#include "DialogBuilderSetting.h"
#include "QuestData.h"


#define LOCTEXT_NAMESPACE "FDialog_System_EditorModule"

static const FName Dialog_System_EditorTabName("Dialog_System_Editor");
TSharedPtr<FSlateStyleSet> FDialog_System_EditorModule::CustomAssetsEditorSlateStyle;

void FDialog_System_EditorModule::StartupModule()
{	
	// Register the PropertyEditors
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	//Initialize
	FDialogBuilder_EditorStyle::Initialize();
	FDialogBuilder_EditorStyle::ReloadTextures();
	FDialogBuilder_EditorCommands::Register();
		
	//Register Settings Menu For Dialog Builder
	RegisterSettings();

	//Register GraphNodeFactory
	GraphPanelNodeFactory_DialogEditor = MakeShareable(new FDialogBuilderNodeFactory());
	FEdGraphUtilities::RegisterVisualNodeFactory(GraphPanelNodeFactory_DialogEditor);
	

	// Register the slate styles	
	const FString ProjectResourceDir = FPaths::ProjectPluginsDir() / TEXT("DialogBuilder/Resources");
	CustomAssetsEditorSlateStyle = MakeShareable(new FSlateStyleSet("DialogEditor"));
	CustomAssetsEditorSlateStyle->Set("CustomAsset.Thumbnail", new FSlateImageBrush(FString(ProjectResourceDir / "Icon128.png"), FVector2D(128.0f, 128.0f)));
	CustomAssetsEditorSlateStyle->Set("ClassIcon.CustomActor", new FSlateImageBrush(FString(ProjectResourceDir / "Icon128.png"), FVector2D(32.0f, 32.0f)));
	CustomAssetsEditorSlateStyle->Set("ClassIcon.UAssetDefinition_CustomObject", new FSlateImageBrush(FString(ProjectResourceDir / "Icon128.png"), FVector2D(32.0f, 32.0f)));

	FSlateStyleRegistry::RegisterSlateStyle(*CustomAssetsEditorSlateStyle.Get());
}

void FDialog_System_EditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	if (GraphPanelNodeFactory_DialogEditor.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(GraphPanelNodeFactory_DialogEditor);
		GraphPanelNodeFactory_DialogEditor.Reset();
	}

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FDialogBuilder_EditorStyle::Shutdown();

	FDialogBuilder_EditorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(Dialog_System_EditorTabName);

	//Unregister PropertyEditor
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	}

	ClassCache.Reset();
}


void FDialog_System_EditorModule::CheckClassCache()
{
	if (!ClassCache.IsValid())
	{
		ClassCache = MakeShareable(new FGraphNodeClassHelper(UQuestTask::StaticClass()));
		FGraphNodeClassHelper::AddObservedBlueprintClasses(UQuestTask::StaticClass());
		ClassCache->UpdateAvailableBlueprintClasses();
	}
}

void FDialog_System_EditorModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		// Register the settings
		SettingsModule->RegisterSettings("Project", "Plugins", "Dialog Builder - Editor",
			LOCTEXT("DialogBuilderSettingsName", "Dialog Builder - Editor"),
			LOCTEXT("DialogBuilderSettingsDescription", "Configuration Settings for the Dialog Builder Editor"),
			GetMutableDefault<UDialogBuilderSetting>()
		);

	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDialog_System_EditorModule, Dialog_System_Editor)

