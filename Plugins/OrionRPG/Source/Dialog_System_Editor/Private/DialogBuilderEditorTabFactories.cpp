// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "DialogBuilderEditorTabFactories.h"

#include "DialogBuilderEditor.h"
#include "DialogBuilderGraph.h"
#include "Containers/Array.h"
#include "Engine/Blueprint.h"
#include "GraphEditor.h"
#include "HAL/PlatformCrt.h"
#include "Internationalization/Internationalization.h"
#include "Math/Vector2D.h"
#include "Misc/AssertionMacros.h"
#include "Styling/AppStyle.h"
#include "Styling/ISlateStyle.h"
#include "Textures/SlateIcon.h"
#include "Widgets/Docking/SDockTab.h"

class SWidget;
struct FSlateBrush;

#define LOCTEXT_NAMESPACE "DialogBuilderEditorTabFactories"

FDialogEditorDetailsSummoner::FDialogEditorDetailsSummoner(TSharedPtr<class FDialogBuilderEditor> InDialogEditorPtr)
	: FWorkflowTabFactory(FDialogBuilderEditorTabs::DialogBuilderPropertyID, InDialogEditorPtr)
	, DialogEditorPtr(InDialogEditorPtr)
{
	TabLabel = LOCTEXT("DialogEditorDetailsLabel", "Details");
	TabIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details");

	bIsSingleton = true;

	ViewMenuDescription = LOCTEXT("DialogEditorDetailsView", "Details");
	ViewMenuTooltip = LOCTEXT("DialogEditorDetailsView_ToolTip", "Show the details view");
}

TSharedRef<SWidget> FDialogEditorDetailsSummoner::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	check(DialogEditorPtr.IsValid());
	return DialogEditorPtr.Pin()->SpawnProperties();
}

FText FDialogEditorDetailsSummoner::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("DialogEditorDetailsTabTooltip", "The dialog editor details tab allows editing of the properties of dialog nodes");
}

FDialogEditorListSummoner::FDialogEditorListSummoner(TSharedPtr<class FDialogBuilderEditor> InDialogEditorPtr)
	: FWorkflowTabFactory(FDialogBuilderEditorTabs::MyDialogDetailID, InDialogEditorPtr)
	, DialogEditorPtr(InDialogEditorPtr)
{
	TabLabel = LOCTEXT("DialogListLabel", "Dialog List");
	TabIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "Kismet.Tabs.FindResults");

	bIsSingleton = true;

	ViewMenuDescription = LOCTEXT("DialogListView", "Dialog List");
	ViewMenuTooltip = LOCTEXT("DialogListView_ToolTip", "Show the dialog list");
}

TSharedRef<SWidget> FDialogEditorListSummoner::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return DialogEditorPtr.Pin()->SpawnDialogList();
}

FText FDialogEditorListSummoner::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("DialogListTabTooltip", "Show the dialog list");
}

FDialogGraphEditorSummoner::FDialogGraphEditorSummoner(TSharedPtr<class FDialogBuilderEditor> InDialogEditorPtr, FOnCreateGraphEditorWidget CreateGraphEditorWidgetCallback)
	: FDocumentTabFactoryForObjects<UEdGraph>("Document", InDialogEditorPtr)
	, DialogEditorPtr(InDialogEditorPtr)
	, OnCreateGraphEditorWidget(CreateGraphEditorWidgetCallback)
{
}

void FDialogGraphEditorSummoner::OnTabActivated(TSharedPtr<SDockTab> Tab) const
{
	check(DialogEditorPtr.IsValid());
	TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Tab->GetContent());
	DialogEditorPtr.Pin()->OnGraphEditorFocused(GraphEditor);
}

void FDialogGraphEditorSummoner::OnTabRefreshed(TSharedPtr<SDockTab> Tab) const
{
	TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Tab->GetContent());
	GraphEditor->NotifyGraphChanged();
}

TAttribute<FText> FDialogGraphEditorSummoner::ConstructTabNameForObject(UEdGraph* DocumentID) const
{
	return TAttribute<FText>( FText::FromString( DocumentID->GetName() ) );
}

TSharedRef<SWidget> FDialogGraphEditorSummoner::CreateTabBodyForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* DocumentID) const
{
	return OnCreateGraphEditorWidget.Execute(DocumentID);
}

const FSlateBrush* FDialogGraphEditorSummoner::GetTabIconForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* DocumentID) const
{
	return FAppStyle::Get().GetBrush("BTEditor.BehaviorTree");
}

void FDialogGraphEditorSummoner::SaveState(TSharedPtr<SDockTab> Tab, TSharedPtr<FTabPayload> Payload) const
{
	check(DialogEditorPtr.IsValid());
	check(DialogEditorPtr.Pin()->GetDialogBuilderGraph());

	TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Tab->GetContent());

	FVector2f ViewLocation;
	float ZoomAmount;
	GraphEditor->GetViewLocation(ViewLocation, ZoomAmount);

	UEdGraph* Graph = FTabPayload_UObject::CastChecked<UEdGraph>(Payload);
	DialogEditorPtr.Pin()->GetDialogBuilderGraph()->LastEditedDocuments.Add(FEditedDocumentInfo(Graph, ViewLocation, ZoomAmount));
}

#undef LOCTEXT_NAMESPACE

