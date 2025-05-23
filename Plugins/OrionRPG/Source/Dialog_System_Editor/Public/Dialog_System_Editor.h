// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIGraphTypes.h"
#include "Modules/ModuleManager.h"
#include <EdGraphUtilities.h>
#include "SSubobjectEditor.h"
#include <IAssetTools.h>
#include <DialogBuilderNodeFactory.h>

class FToolBarBuilder;
class FMenuBuilder;
class UDialog;



class FDialog_System_EditorModule : public IModuleInterface
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



	class UDialogEditorSettings* SettingsPtr;
private:
	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedPtr<FDialogBuilderNodeFactory> GraphPanelNodeFactory_DialogEditor;
	TSharedPtr<class IDetailsView> PropertyWidget;

	EAssetTypeCategories::Type DialogEditorAssetCategoryBit;
	TArray< TSharedPtr<IAssetTypeActions> > CreatedAssetTypeActions;

	TSharedPtr<struct FGraphNodeClassHelper> DecoratorClassCache;
	TSharedPtr<struct FGraphNodeClassHelper> EventClassCache;
};
