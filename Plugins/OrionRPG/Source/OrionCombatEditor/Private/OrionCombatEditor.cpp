// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "OrionCombatEditor.h"
#include "IAssetTools.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "ISettingsContainer.h"
#include <ISettingsCategory.h>
#include "LevelEditor.h"
#include "OrionCombatSetting.h"
#include "OrionCombat_EditorCommands.h"
#include "OrionCombat_EditorStyle.h"
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
	FOrionCombat_EditorStyle::Initialize();
	FOrionCombat_EditorStyle::ReloadTextures();
	FOrionCombat_EditorCommands::Register();

	//Register Settings Menu For Dialog Builder
	RegisterSettings();
	
	PropertyModule.NotifyCustomizationModuleChanged();

}

void FOrionCombatEditorModule::ShutdownModule()
{
	MenuExtensibilityManager.Reset();
	ToolBarExtensibilityManager.Reset();

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FOrionCombat_EditorStyle::Shutdown();

	FOrionCombat_EditorCommands::Unregister();

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

}




void FOrionCombatEditorModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		// Register the settings
		SettingsModule->RegisterSettings("Project", "Game", "OrionRPG - Combat",
			LOCTEXT("OrionCombatSettingsName", "OrionRPG - Combat"),
			LOCTEXT("OrionCombatSettingsDescription", "Configuration Settings for the Orion Combat"),
			GetMutableDefault<UOrionCombatSetting>()
		);

	}
}

void FOrionCombatEditorModule::RegisterObjectCustomizations()
{
	//RegisterCustomClassLayout("OrionCombatNode_DialogLine", FOnGetDetailCustomizationInstance::CreateStatic(&FDialogNodeDetails::MakeInstance));

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

