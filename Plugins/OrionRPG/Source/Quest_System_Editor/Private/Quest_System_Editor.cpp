// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Quest_System_Editor.h"
#include "QuestBuilder_EditorStyle.h"
#include "QuestBuilder_EditorCommands.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderNode_Objective.h"
#include "QuestBuilderSetting.h"
#include "Task/QuestTask.h"
#include "Decorator/OrionDecorator.h"
#include "Event/OrionEvent.h"
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
#include "QuestBuilderSetting.h"
#include "QuestData.h"


#define LOCTEXT_NAMESPACE "FQuest_System_EditorModule"

static const FName Quest_System_EditorTabName("Quest_System_Editor");
TSharedPtr<FSlateStyleSet> FQuest_System_EditorModule::CustomAssetsEditorSlateStyle;

void FQuest_System_EditorModule::StartupModule()
{	
	// Register the PropertyEditors
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	//Initialize
	FQuestBuilder_EditorStyle::Initialize();
	FQuestBuilder_EditorStyle::ReloadTextures();
	FQuestBuilder_EditorCommands::Register();

	//Register Settings Menu For Quest Builder
	RegisterSettings();

	//Register GraphNodeFactory
	GraphPanelNodeFactory_QuestEditor = MakeShareable(new FQuestBuilderNodeFactory());
	FEdGraphUtilities::RegisterVisualNodeFactory(GraphPanelNodeFactory_QuestEditor);
	

	// Register the slate styles	
	const FString ProjectResourceDir = FPaths::ProjectPluginsDir() / TEXT("OrionRPG/Resources");
	CustomAssetsEditorSlateStyle = MakeShareable(new FSlateStyleSet("QuestEditor"));
	CustomAssetsEditorSlateStyle->Set("QuestBuilder.Thumbnail", new FSlateImageBrush(FString(ProjectResourceDir / "Icon128.png"), FVector2D(128.0f, 128.0f)));
	CustomAssetsEditorSlateStyle->Set("ClassIcon.QuestBuilder", new FSlateImageBrush(FString(ProjectResourceDir / "Icon128.png"), FVector2D(32.0f, 32.0f)));
	CustomAssetsEditorSlateStyle->Set("ClassIcon.UAssetDefinition_QuestBuilder", new FSlateImageBrush(FString(ProjectResourceDir / "Icon128.png"), FVector2D(32.0f, 32.0f)));

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

	FQuestBuilder_EditorStyle::Shutdown();

	FQuestBuilder_EditorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(Quest_System_EditorTabName);

	//Unregister PropertyEditor
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	}

	ObjectiveClassCache.Reset();
	DecoratorClassCache.Reset();
	EventClassCache.Reset();
}


void FQuest_System_EditorModule::CheckClassCache()
{
	if (!ObjectiveClassCache.IsValid())
	{
		ObjectiveClassCache = MakeShareable(new FGraphNodeClassHelper(UQuestBuilderNode_Objective::StaticClass()));
		FGraphNodeClassHelper::AddObservedBlueprintClasses(UQuestBuilderNode_Objective::StaticClass());
		ObjectiveClassCache->UpdateAvailableBlueprintClasses();
	}
	if (!DecoratorClassCache.IsValid())
	{
		DecoratorClassCache = MakeShareable(new FGraphNodeClassHelper(UOrionDecorator::StaticClass()));
		FGraphNodeClassHelper::AddObservedBlueprintClasses(UOrionDecorator::StaticClass());
		DecoratorClassCache->UpdateAvailableBlueprintClasses();
	}
	if (!EventClassCache.IsValid())
	{
		EventClassCache = MakeShareable(new FGraphNodeClassHelper(UOrionEvent::StaticClass()));
		FGraphNodeClassHelper::AddObservedBlueprintClasses(UOrionEvent::StaticClass());
		EventClassCache->UpdateAvailableBlueprintClasses();
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
			GetMutableDefault<UQuestBuilderSetting>()
		);

	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FQuest_System_EditorModule, Quest_System_Editor)

