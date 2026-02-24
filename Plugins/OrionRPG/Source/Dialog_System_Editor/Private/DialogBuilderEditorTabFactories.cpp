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


FDialogSequencerTabSummoner::FDialogSequencerTabSummoner(TSharedPtr<class FDialogBuilderEditor> InDialogEditorPtr)
	: FWorkflowTabFactory(FDialogBuilderEditorTabs::DialogSequencerTabID, InDialogEditorPtr)
	, DialogEditorPtr(InDialogEditorPtr)
{
	TabLabel = LOCTEXT("DialogSequencerTabLabel", "Dialog Sequencer");
	TabIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Cinematics");

	bIsSingleton = true;

	ViewMenuDescription = LOCTEXT("DialogSequencerTabView", "Dialog Sequencer");
	ViewMenuTooltip = LOCTEXT("DialogSequencerTabView_ToolTip", "Show the dialog sequencer tab");
}

TSharedRef<SWidget> FDialogSequencerTabSummoner::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return DialogEditorPtr.Pin()->SpawnDialogSequencerTab();
}

FText FDialogSequencerTabSummoner::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("DialogSequencerTabToolTip", "Show the dialog sequencer tab");
}


#undef LOCTEXT_NAMESPACE

