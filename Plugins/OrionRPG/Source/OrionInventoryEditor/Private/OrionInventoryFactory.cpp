// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "OrionInventoryFactory.h"
#include "Inventory/OrionInventoryGraph.h"
#include "OrionInventoryEditor.h"
#include "ClassViewerModule.h"
#include "ClassViewerFilter.h"
#include "OrionInventoryEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/SClassPickerDialog.h"

#define LOCTEXT_NAMESPACE "Inventory_Editor"

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

UOrionInventoryFactory::UOrionInventoryFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SupportedClass = UOrionInventoryGraph::StaticClass();
	InventorySystemGraphClass = UOrionInventoryGraph::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}


bool UOrionInventoryFactory::ConfigureProperties()
{
	return true;
}

UObject* UOrionInventoryFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UOrionInventoryGraph::StaticClass()));
	UOrionInventoryGraph* InventoryGraph = nullptr;
	if (InventorySystemGraphClass)
	{
		InventoryGraph =  NewObject<UOrionInventoryGraph>(InParent, InventorySystemGraphClass, Name, Flags | RF_Transactional);
		InventoryGraph->ID = FOrionInventoryEditorUtils::FindUniqueInventoryGraphName("OrionInventoryAsset");
		
	}

	return InventoryGraph;
}

FInventoryGraphEditorSummoner::FInventoryGraphEditorSummoner(TSharedPtr<class FOrionInventoryEditor> InInventoryEditorPtr, FOnCreateGraphEditorWidget CreateGraphEditorWidgetCallback) : FDocumentTabFactoryForObjects<UEdGraph>(FOrionInventoryEditorTabs::ViewportID, InInventoryEditorPtr)
, InventoryEditorPtr(InInventoryEditorPtr)
, OnCreateGraphEditorWidget(CreateGraphEditorWidgetCallback)
{
}

void FInventoryGraphEditorSummoner::OnTabActivated(TSharedPtr<SDockTab> Tab) const
{
	TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Tab->GetContent());
	InventoryEditorPtr.Pin()->OnGraphEditorFocused(GraphEditor);
}

void FInventoryGraphEditorSummoner::OnTabBackgrounded(TSharedPtr<SDockTab> Tab) const
{
	TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Tab->GetContent());
	InventoryEditorPtr.Pin()->OnGraphEditorBackgrounded(GraphEditor);
}

void FInventoryGraphEditorSummoner::OnTabRefreshed(TSharedPtr<SDockTab> Tab) const
{
	TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Tab->GetContent());
	GraphEditor->NotifyGraphChanged();
}

void FInventoryGraphEditorSummoner::SaveState(TSharedPtr<SDockTab> Tab, TSharedPtr<FTabPayload> Payload) const
{
	TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Tab->GetContent());

#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
	FVector2f ViewLocation;
#else if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 5)
	FVector2D ViewLocation;
#endif
	float ZoomAmount;
	GraphEditor->GetViewLocation(ViewLocation, ZoomAmount);

	UEdGraph* Graph = Payload->IsValid() ? FTabPayload_UObject::CastChecked<UEdGraph>(Payload) : nullptr;

	if (Graph && InventoryEditorPtr.Pin()->IsGraphInCurrentInventoryGraph(Graph))
	{
		// Don't save references to external graphs.
		InventoryEditorPtr.Pin()->GetOrionInventoryGraph()->LastEditedDocuments.Add(FEditedDocumentInfo(Graph, ViewLocation, ZoomAmount));
	}
}

TSharedRef<SWidget> FInventoryGraphEditorSummoner::CreateTabBodyForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* DocumentID) const
{
	check(Info.TabInfo.IsValid());
	return OnCreateGraphEditorWidget.Execute(Info.TabInfo.ToSharedRef(), DocumentID);
}

const FSlateBrush* FInventoryGraphEditorSummoner::GetTabIconForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* DocumentID) const
{
	return FOrionInventoryEditor::GetGlyphForGraph(DocumentID, false);
}


EAssetCommandResult UAssetDefinition_InventoryEditor::OpenAssets(const FAssetOpenArgs& OpenArgs) const
{
	const EToolkitMode::Type Mode = OpenArgs.ToolkitHost.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
	for (UOrionInventoryGraph* InventoryGraph : OpenArgs.LoadObjects<UOrionInventoryGraph>())
	{
		FOrionInventoryEditorModule& InventorySystemEditorModule = FModuleManager::GetModuleChecked<FOrionInventoryEditorModule>("OrionInventoryEditor");
		InventorySystemEditorModule.CheckClassCache();

		const TSharedRef<FOrionInventoryEditor> CustomObjectToolkit(new FOrionInventoryEditor());
		CustomObjectToolkit->Initialize(Mode, OpenArgs.ToolkitHost, InventoryGraph);
	}

	return EAssetCommandResult::Handled;
}

#undef LOCTEXT_NAMESPACE

