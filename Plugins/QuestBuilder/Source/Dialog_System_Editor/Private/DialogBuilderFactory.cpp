// Copyright 2024 Ivan Chandra. All Rights Reserved.

#include "DialogBuilderFactory.h"
#include "QuestBuilderGraph.h"
#include "DialogBuilderEditor.h"
#include "ClassViewerModule.h"
#include "ClassViewerFilter.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/SClassPickerDialog.h"

#define LOCTEXT_NAMESPACE "Quest_Editor"

class FAssetClassParentFilter : public IClassViewerFilter
{
public:
	FAssetClassParentFilter()
		: DisallowedClassFlags(CLASS_None), bDisallowBlueprintBase(false)
	{}

	/** All children of these classes will be included unless filtered out by another setting. */
	TSet< const UClass* > AllowedChildrenOfClasses;

	/** Disallowed class flags. */
	EClassFlags DisallowedClassFlags;

	/** Disallow blueprint base classes. */
	bool bDisallowBlueprintBase;

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		bool bAllowed = !InClass->HasAnyClassFlags(DisallowedClassFlags)
			&& InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InClass) != EFilterReturn::Failed;

		if (bAllowed && bDisallowBlueprintBase)
		{
			if (FKismetEditorUtilities::CanCreateBlueprintOfClass(InClass))
			{
				return false;
			}
		}

		return bAllowed;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		if (bDisallowBlueprintBase)
		{
			return false;
		}

		return !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags)
			&& InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
	}
};

UDialogBuilderFactory::UDialogBuilderFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SupportedClass = UQuestBuilderGraph::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}


bool UDialogBuilderFactory::ConfigureProperties()
{
	return true;
}

UObject* UDialogBuilderFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	if (QuestSystemGraphClass != nullptr)
	{
		return NewObject<UQuestBuilderGraph>(InParent, QuestSystemGraphClass, Name, Flags | RF_Transactional);
	}
	else
	{
		check(Class->IsChildOf(UQuestBuilderGraph::StaticClass()));
		return NewObject<UObject>(InParent, Class, Name, Flags | RF_Transactional);
	}
}

FDialogGraphEditorSummoner::FDialogGraphEditorSummoner(TSharedPtr<class FDialogBuilderEditor> InQuestEditorPtr, FOnCreateGraphEditorWidget CreateGraphEditorWidgetCallback) : FDocumentTabFactoryForObjects<UEdGraph>(FDialogBuilderEditorTabs::ViewportID, InQuestEditorPtr)
, QuestEditorPtr(InQuestEditorPtr)
, OnCreateGraphEditorWidget(CreateGraphEditorWidgetCallback)
{
}

void FDialogGraphEditorSummoner::OnTabActivated(TSharedPtr<SDockTab> Tab) const
{
	TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Tab->GetContent());
	QuestEditorPtr.Pin()->OnGraphEditorFocused(GraphEditor);
}

void FDialogGraphEditorSummoner::OnTabBackgrounded(TSharedPtr<SDockTab> Tab) const
{
	TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Tab->GetContent());
	QuestEditorPtr.Pin()->OnGraphEditorBackgrounded(GraphEditor);
}

void FDialogGraphEditorSummoner::OnTabRefreshed(TSharedPtr<SDockTab> Tab) const
{
	TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Tab->GetContent());
	GraphEditor->NotifyGraphChanged();
}

void FDialogGraphEditorSummoner::SaveState(TSharedPtr<SDockTab> Tab, TSharedPtr<FTabPayload> Payload) const
{
	TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Tab->GetContent());

	FVector2D ViewLocation;
	float ZoomAmount;
	GraphEditor->GetViewLocation(ViewLocation, ZoomAmount);

	UEdGraph* Graph = Payload->IsValid() ? FTabPayload_UObject::CastChecked<UEdGraph>(Payload) : nullptr;

	if (Graph && QuestEditorPtr.Pin()->IsGraphInCurrentQuestGraph(Graph))
	{
		// Don't save references to external graphs.
		QuestEditorPtr.Pin()->GetQuestBuilderGraph()->LastEditedDocuments.Add(FEditedDocumentInfo(Graph, ViewLocation, ZoomAmount));
	}
}

TSharedRef<SWidget> FDialogGraphEditorSummoner::CreateTabBodyForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* DocumentID) const
{
	check(Info.TabInfo.IsValid());
	return OnCreateGraphEditorWidget.Execute(Info.TabInfo.ToSharedRef(), DocumentID);
}

const FSlateBrush* FDialogGraphEditorSummoner::GetTabIconForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* DocumentID) const
{
	return FDialogBuilderEditor::GetGlyphForGraph(DocumentID, false);
}


EAssetCommandResult UAssetDefinition_DialogBuilder::OpenAssets(const FAssetOpenArgs& OpenArgs) const
{
	const EToolkitMode::Type Mode = OpenArgs.ToolkitHost.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
	for (UQuestBuilderGraph* QuestGraph : OpenArgs.LoadObjects<UQuestBuilderGraph>())
	{
		FDialog_System_EditorModule& QuestSystemEditorModule = FModuleManager::GetModuleChecked<FDialog_System_EditorModule>("Dialog_System_Editor");
		QuestSystemEditorModule.CheckClassCache();

		const TSharedRef<FDialogBuilderEditor> CustomObjectToolkit(new FDialogBuilderEditor());
		CustomObjectToolkit->Initialize(Mode, OpenArgs.ToolkitHost, QuestGraph);
	}

	return EAssetCommandResult::Handled;
}

#undef LOCTEXT_NAMESPACE

