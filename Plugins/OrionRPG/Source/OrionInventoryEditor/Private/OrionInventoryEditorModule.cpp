// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "OrionInventoryEditorModule.h"
#include "OrionInventory_EditorStyle.h"
#include "OrionInventory_EditorCommands.h"
#include "Inventory/OrionInventoryNode_Item.h"
#include "Inventory/OrionInventorySetting.h"
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


#define LOCTEXT_NAMESPACE "FOrionInventoryEditorModule"

static const FName OrionInventoryEditorTabName("OrionInventoryEditor");
TSharedPtr<FSlateStyleSet> FOrionInventoryEditorModule::CustomAssetsEditorSlateStyle;

void FOrionInventoryEditorModule::StartupModule()
{	
	// Register the PropertyEditors
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	//Initialize
	FOrionInventory_EditorStyle::Initialize();
	FOrionInventory_EditorStyle::ReloadTextures();
	FOrionInventory_EditorCommands::Register();

	//Register Settings Menu For Inventory Builder
	RegisterSettings();

	//Register GraphNodeFactory
	GraphPanelNodeFactory_InventoryEditor = MakeShareable(new FOrionInventoryNodeFactory());
	FEdGraphUtilities::RegisterVisualNodeFactory(GraphPanelNodeFactory_InventoryEditor);
	

	// Register the slate styles	
	const FString ProjectResourceDir = FPaths::ProjectPluginsDir() / TEXT("OrionRPG/Resources");
	CustomAssetsEditorSlateStyle = MakeShareable(new FSlateStyleSet("InventoryEditor"));
	CustomAssetsEditorSlateStyle->Set("OrionInventory.Thumbnail", new FSlateImageBrush(FString(ProjectResourceDir / "InventoryIcon128.png"), FVector2D(128.0f, 128.0f)));
	CustomAssetsEditorSlateStyle->Set("ClassIcon.OrionInventory", new FSlateImageBrush(FString(ProjectResourceDir / "InventoryIcon128.png"), FVector2D(32.0f, 32.0f)));
	CustomAssetsEditorSlateStyle->Set("ClassIcon.UAssetDefinition_OrionInventory", new FSlateImageBrush(FString(ProjectResourceDir / "InventoryIcon128.png"), FVector2D(32.0f, 32.0f)));

	FSlateStyleRegistry::RegisterSlateStyle(*CustomAssetsEditorSlateStyle.Get());
}

void FOrionInventoryEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	if (GraphPanelNodeFactory_InventoryEditor.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(GraphPanelNodeFactory_InventoryEditor);
		GraphPanelNodeFactory_InventoryEditor.Reset();
	}

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FOrionInventory_EditorStyle::Shutdown();

	FOrionInventory_EditorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(OrionInventoryEditorTabName);

	//Unregister PropertyEditor
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	}

	InventoryItemClassCache.Reset();
}


void FOrionInventoryEditorModule::CheckClassCache()
{
	if (!InventoryItemClassCache.IsValid())
	{
		InventoryItemClassCache = MakeShareable(new FGraphNodeClassHelper(UOrionInventoryNode_Item::StaticClass()));
		FGraphNodeClassHelper::AddObservedBlueprintClasses(UOrionInventoryNode_Item::StaticClass());
		InventoryItemClassCache->UpdateAvailableBlueprintClasses();
	}

}

void FOrionInventoryEditorModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		// Register the settings
		SettingsModule->RegisterSettings("Project", "Plugins", "OrionRPG - Inventory",
			LOCTEXT("OrionInventorySettingsName", "Inventory Builder - Editor"),
			LOCTEXT("OrionInventorySettingsDescription", "Configuration Settings for the Inventory Builder Editor"),
			GetMutableDefault<UOrionInventorySetting>()
		);

	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOrionInventoryEditorModule, OrionInventoryEditor)

