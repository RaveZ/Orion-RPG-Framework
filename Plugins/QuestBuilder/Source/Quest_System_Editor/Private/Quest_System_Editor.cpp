// Copyright 2024 Ivan Chandra. All Rights Reserved.

#include "Quest_System_Editor.h"
#include "Quest_System_EditorStyle.h"
#include "Quest_System_EditorCommands.h"
#include "QuestSystemGraphNode.h"
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
#include "Setting_QuestEditor.h"
#include "AttributeDetails.h"
#include "QuestData.h"


#define LOCTEXT_NAMESPACE "FQuest_System_EditorModule"

static const FName Quest_System_EditorTabName("Quest_System_Editor");
TSharedPtr<FSlateStyleSet> FQuest_System_EditorModule::CustomAssetsEditorSlateStyle;

void FQuest_System_EditorModule::StartupModule()
{	
	// Register the PropertyEditors
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	//PropertyModule.RegisterCustomPropertyTypeLayout(FQuestDataHandle::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FQuestPropertyDetails::MakeInstance));

	//Initialize
	FQuest_System_EditorStyle::Initialize();
	FQuest_System_EditorStyle::ReloadTextures();
	FQuest_System_EditorCommands::Register();

	//Register Settings Menu For Quest Builder
	RegisterSettings();

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

	//Unregister PropertyEditor
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomPropertyTypeLayout(FQuestDataHandle::StaticStruct()->GetFName());
	}

	ClassCache.Reset();
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

void FQuest_System_EditorModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		// Register the settings
		SettingsModule->RegisterSettings("Project", "Plugins", "Quest Builder - Editor",
			LOCTEXT("QuestBuilderSettingsName", "Quest Builder - Editor"),
			LOCTEXT("QuestBuilderSettingsDescription", "Configuration Settings for the Quest Builder Editor"),
			GetMutableDefault<UQuestEditorSettings>()
		);

	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FQuest_System_EditorModule, Quest_System_Editor)

