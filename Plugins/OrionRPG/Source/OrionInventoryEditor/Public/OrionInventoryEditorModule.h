// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIGraphTypes.h"
#include "Modules/ModuleManager.h"
#include <EdGraphUtilities.h>
#include "SSubobjectEditor.h"
#include <IAssetTools.h>
#include <OrionInventoryNodeFactory.h>

class FToolBarBuilder;
class FMenuBuilder;
class UInventory;



class FOrionInventoryEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	
	TSharedPtr<struct FGraphNodeClassHelper> GetDecoratorClassCache() { return DecoratorClassCache; }
	TSharedPtr<struct FGraphNodeClassHelper> GetEventClassCache() { return EventClassCache; }
	
	void CheckClassCache();

	static TSharedPtr<FSlateStyleSet> CustomAssetsEditorSlateStyle;

private:

	void RegisterSettings();



	class UInventoryEditorSettings* SettingsPtr;
private:
	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedPtr<FOrionInventoryNodeFactory> GraphPanelNodeFactory_InventoryEditor;
	TSharedPtr<class IDetailsView> PropertyWidget;

	EAssetTypeCategories::Type InventoryEditorAssetCategoryBit;
	TArray< TSharedPtr<IAssetTypeActions> > CreatedAssetTypeActions;

	TSharedPtr<struct FGraphNodeClassHelper> DecoratorClassCache;
	TSharedPtr<struct FGraphNodeClassHelper> EventClassCache;
};
