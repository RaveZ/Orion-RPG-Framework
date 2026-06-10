// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIGraphTypes.h"
#include "Modules/ModuleManager.h"
#include "EdGraphUtilities.h"
#include "SSubobjectEditor.h"
#include "IAssetTools.h"
#include "PropertyEditorDelegates.h"
#include "Templates/SharedPointer.h"
#include "Toolkits/AssetEditorToolkit.h"

class FToolBarBuilder;
class FMenuBuilder;
class UDialog;



class FOrionCombatEditorModule : public IModuleInterface, public IHasMenuExtensibility, public IHasToolBarExtensibility
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	

	/** Gets the extensibility managers for outside entities to extend static mesh editor's menus and toolbars */
	virtual TSharedPtr<FExtensibilityManager> GetMenuExtensibilityManager() override { return MenuExtensibilityManager; }
	virtual TSharedPtr<FExtensibilityManager> GetToolBarExtensibilityManager() override { return ToolBarExtensibilityManager; }
	
	static TSharedPtr<FSlateStyleSet> CustomAssetsEditorSlateStyle;

private:
	void RegisterSettings();
	void RegisterObjectCustomizations();
	void RegisterPropertyTypeCustomizations();

	/**
	 * Registers a custom class
	 *
	 * @param ClassName				The class name to register for property customization
	 * @param DetailLayoutDelegate	The delegate to call to get the custom detail layout instance
	 */
	void RegisterCustomClassLayout(FName ClassName, FOnGetDetailCustomizationInstance DetailLayoutDelegate);
	void RegisterCustomPropertyTypeLayout(FName PropertyTypeName, FOnGetPropertyTypeCustomizationInstance PropertyTypeLayoutDelegate);



	class UDialogEditorSettings* SettingsPtr;
private:
	TSharedPtr<FExtensibilityManager> MenuExtensibilityManager;
	TSharedPtr<FExtensibilityManager> ToolBarExtensibilityManager;

	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedPtr<class IDetailsView> PropertyWidget;

	EAssetTypeCategories::Type OrionRPGAssetCategoryBit;
	TArray< TSharedPtr<IAssetTypeActions> > CreatedAssetTypeActions;

	/** List of registered class that we must unregister when the module shuts down */
	TSet< FName > RegisteredClassNames;
	TSet< FName > RegisteredPropertyTypeNames;
private:
	FDelegateHandle DialogTrackEditorHandle;
};
