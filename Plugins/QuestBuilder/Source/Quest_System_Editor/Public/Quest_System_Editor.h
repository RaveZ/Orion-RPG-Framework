// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIGraphTypes.h"
#include "Modules/ModuleManager.h"
#include <EdGraphUtilities.h>
#include "SSubobjectEditor.h"
#include <IAssetTools.h>
#include <QuestBuilderNodeFactory.h>

class FToolBarBuilder;
class FMenuBuilder;
class UQuest;



class FQuest_System_EditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	
	TSharedPtr<struct FGraphNodeClassHelper> GetClassCache() { return ClassCache; }
	
	void CheckClassCache();

	static TSharedPtr<FSlateStyleSet> CustomAssetsEditorSlateStyle;

private:

	void RegisterSettings();



	class UQuestEditorSettings* SettingsPtr;
private:
	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedPtr<FQuestBuilderNodeFactory> GraphPanelNodeFactory_QuestEditor;
	TSharedPtr<class IDetailsView> PropertyWidget;

	EAssetTypeCategories::Type QuestEditorAssetCategoryBit;
	TArray< TSharedPtr<IAssetTypeActions> > CreatedAssetTypeActions;

	TSharedPtr<struct FGraphNodeClassHelper> ClassCache;
};
