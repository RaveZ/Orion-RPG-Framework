// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "OrionCombatEditor/Public/OrionCombatEditor.h"
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
#include "Modules/ModuleManager.h"


#define LOCTEXT_NAMESPACE "FOrionCombatEditorModule"

static const FName OrionCombatEditorTabName("OrionCombatEditor");
TSharedPtr<FSlateStyleSet> FOrionCombatEditorModule::CustomAssetsEditorSlateStyle;

void FOrionCombatEditorModule::StartupModule()
{
	MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);
	ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager);

	// Register the PropertyEditors
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	RegisterObjectCustomizations();

	//Initialize
	FDialogBuilder_EditorStyle::Initialize();
	FDialogBuilder_EditorStyle::ReloadTextures();
	FDialogBuilder_EditorCommands::Register();

	//Register Settings Menu For Dialog Builder
	RegisterSettings();

	FEdGraphUtilities::RegisterVisualNodeFactory(GraphPanelNodeFactory_DialogEditor);
	
	PropertyModule.NotifyCustomizationModuleChanged();

	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer");
	DialogTrackEditorHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FDialogTrackEditor::CreateTrackEditor));
}

void FOrionCombatEditorModule::ShutdownModule()
{
	MenuExtensibilityManager.Reset();
	ToolBarExtensibilityManager.Reset();

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

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(OrionCombatEditorTabName);

	//Unregister PropertyEditor
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

		// Unregister all classes customized by name
		for (auto It = RegisteredClassNames.CreateConstIterator(); It; ++It)
		{
			if (It->IsValid())
			{
				PropertyModule.UnregisterCustomClassLayout(*It);
			}
		}
	}

	if (FModuleManager::Get().IsModuleLoaded("Sequencer"))
	{
		ISequencerModule& SequencerModule = FModuleManager::GetModuleChecked<ISequencerModule>("Sequencer");
		SequencerModule.UnRegisterTrackEditor(DialogTrackEditorHandle);
	}

	DecoratorClassCache.Reset();
	EventClassCache.Reset();
}


void FOrionCombatEditorModule::CheckClassCache()
{
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

void FOrionCombatEditorModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		// Register the settings
		SettingsModule->RegisterSettings("Project", "Game", "OrionRPG - Dialog",
			LOCTEXT("DialogBuilderSettingsName", "OrionRPG - Dialog"),
			LOCTEXT("DialogBuilderSettingsDescription", "Configuration Settings for the Dialog Builder Editor"),
			GetMutableDefault<UDialogBuilderSetting>()
		);

	}
}

void FOrionCombatEditorModule::RegisterObjectCustomizations()
{
	RegisterCustomClassLayout("DialogBuilderNode_DialogLine", FOnGetDetailCustomizationInstance::CreateStatic(&FDialogNodeDetails::MakeInstance));
	RegisterCustomClassLayout("DialogBuilderNode_DialogSequence", FOnGetDetailCustomizationInstance::CreateStatic(&FDialogNodeDetails::MakeInstance));
	RegisterCustomClassLayout("DialogSequenceSlot", FOnGetDetailCustomizationInstance::CreateStatic(&FDialogSequenceDetails::MakeInstance));
	RegisterCustomClassLayout("MovieSceneDialogSection", FOnGetDetailCustomizationInstance::CreateStatic(&FDialogSequenceDetails::MakeInstance));
	RegisterCustomClassLayout("DialogSequenceShot", FOnGetDetailCustomizationInstance::CreateStatic(&FDialogSequenceDetails::MakeInstance));
	RegisterCustomClassLayout("DialogDefinition", FOnGetDetailCustomizationInstance::CreateStatic(&FDialogSequenceDetails::MakeInstance));

}

void FOrionCombatEditorModule::RegisterCustomClassLayout(FName ClassName, FOnGetDetailCustomizationInstance DetailLayoutDelegate)
{
	check(ClassName != NAME_None);

	RegisteredClassNames.Add(ClassName);

	static FName PropertyEditor("PropertyEditor");
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(PropertyEditor);
	PropertyModule.RegisterCustomClassLayout(ClassName, DetailLayoutDelegate);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOrionCombatEditorModule, OrionCombatEditor)

