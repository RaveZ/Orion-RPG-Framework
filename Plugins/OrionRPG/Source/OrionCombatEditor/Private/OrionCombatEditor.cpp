// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "OrionCombatEditor.h"
#include "IAssetTools.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "ISettingsContainer.h"
#include <ISettingsCategory.h>
#include "LevelEditor.h"
#include "OrionCombatAssetFactories.h"
#include "OrionAttributeSetDetails.h"
#include "OrionCombatSetting.h"
#include "OrionCombat_EditorCommands.h"
#include "OrionCombat_EditorStyle.h"
#include "OrionGameplayAttributeDataDetails.h"
#include "OrionGameplayAttributeGraphPanelPinFactory.h"
#include "OrionGameplayAttributeDetails.h"
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
	RegisterPropertyTypeCustomizations();
	GameplayAttributePinFactory = MakeShared<FOrionGameplayAttributeGraphPanelPinFactory>();
	FEdGraphUtilities::RegisterVisualPinFactory(GameplayAttributePinFactory);

	//Initialize
	FOrionCombat_EditorStyle::Initialize();
	FOrionCombat_EditorStyle::ReloadTextures();
	FOrionCombat_EditorCommands::Register();

	//Register Settings Menu For Dialog Builder
	RegisterSettings();

	IAssetTools& AssetTools = IAssetTools::Get();
	OrionRPGAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(
		TEXT("OrionRPG"),
		LOCTEXT("OrionRPGAssetCategory", "Orion RPG")
	);
	OrionCombatEditor::SetOrionRPGAssetCategory(OrionRPGAssetCategoryBit);
	
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

	if (GameplayAttributePinFactory.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualPinFactory(GameplayAttributePinFactory);
		GameplayAttributePinFactory.Reset();
	}

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

		for (auto It = RegisteredPropertyTypeNames.CreateConstIterator(); It; ++It)
		{
			if (It->IsValid())
			{
				PropertyModule.UnregisterCustomPropertyTypeLayout(*It);
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
	RegisterCustomClassLayout("OrionAttributeSet", FOnGetDetailCustomizationInstance::CreateStatic(&FOrionAttributeSetDetails::MakeInstance));
}

void FOrionCombatEditorModule::RegisterPropertyTypeCustomizations()
{
	RegisterCustomPropertyTypeLayout("GameplayAttribute", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FOrionGameplayAttributeDetails::MakeInstance));
	RegisterCustomPropertyTypeLayout("GameplayAttributeData", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FOrionGameplayAttributeDataDetails::MakeInstance));
	
}

void FOrionCombatEditorModule::RegisterCustomClassLayout(FName ClassName, FOnGetDetailCustomizationInstance DetailLayoutDelegate)
{
	check(ClassName != NAME_None);
	RegisteredClassNames.Add(ClassName);

	static FName PropertyEditor("PropertyEditor");
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(PropertyEditor);
	PropertyModule.RegisterCustomClassLayout(ClassName, DetailLayoutDelegate);
}

void FOrionCombatEditorModule::RegisterCustomPropertyTypeLayout(FName PropertyTypeName, FOnGetPropertyTypeCustomizationInstance PropertyTypeLayoutDelegate)
{
	check(PropertyTypeName != NAME_None);

	RegisteredPropertyTypeNames.Add(PropertyTypeName);

	static FName PropertyEditor("PropertyEditor");
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(PropertyEditor);
	PropertyModule.RegisterCustomPropertyTypeLayout(PropertyTypeName, PropertyTypeLayoutDelegate);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOrionCombatEditorModule, OrionCombatEditor)

