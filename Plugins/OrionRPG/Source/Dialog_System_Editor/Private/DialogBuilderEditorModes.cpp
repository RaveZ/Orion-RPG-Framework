// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "DialogBuilderEditorModes.h"

#include "DialogBuilderEditor.h"
#include "DialogBuilderEditorToolbar.h"
#include "DialogBuilderEditorTabFactories.h"
#include "Framework/Docking/TabManager.h"
#include "Misc/AssertionMacros.h"
#include "Types/SlateEnums.h"

/////////////////////////////////////////////////////
// FDialogBuilderEditorApplicationMode

#define LOCTEXT_NAMESPACE "DialogEditorApplicationMode"

FDialogBuilderEditorApplicationMode::FDialogBuilderEditorApplicationMode(TSharedPtr<class FDialogBuilderEditor> InDialogBuilderEditor)
	: FApplicationMode(FDialogBuilderEditor::DialogEditorMode, FDialogBuilderEditor::GetLocalizedMode)
{
	DialogEditor = InDialogBuilderEditor;

	DialogBuilderEditorTabFactories.RegisterFactory(MakeShareable(new FDialogEditorDetailsSummoner(InDialogBuilderEditor)));
	DialogBuilderEditorTabFactories.RegisterFactory(MakeShareable(new FDialogEditorListSummoner(InDialogBuilderEditor)));
	//DialogBuilderEditorTabFactories.RegisterFactory(MakeShareable(new FBlackboardSummoner(InDialogBuilderEditor)));


	TabLayout = FTabManager::NewLayout( "Standalone_DialogEditor_Layout_v2" )
	->AddArea
	(
		FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.2f)
				->AddTab(FDialogBuilderEditorTabs::MyDialogDetailID, ETabState::ClosedTab)
			)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.7f)
				->AddTab("Document", ETabState::ClosedTab)
				->SetHideTabWell(true)
			)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
				->SetSizeCoefficient(0.2f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.6f)
					->AddTab(FDialogBuilderEditorTabs::DialogBuilderPropertyID, ETabState::OpenedTab)
				)
			)
		)
	);
	
	InDialogBuilderEditor->GetToolbarBuilder()->AddModesToolbar(ToolbarExtender);
	InDialogBuilderEditor->GetToolbarBuilder()->AddDialogSystemToolbar(ToolbarExtender);
}

void FDialogBuilderEditorApplicationMode::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	check(DialogEditor.IsValid());
	TSharedPtr<FDialogBuilderEditor> DialogEditorPtr = DialogEditor.Pin();
	
	DialogEditorPtr->RegisterToolbarTab(InTabManager.ToSharedRef());

	// Mode-specific setup
	DialogEditorPtr->PushTabFactories(DialogBuilderEditorTabFactories);

	FApplicationMode::RegisterTabFactories(InTabManager);
}

void FDialogBuilderEditorApplicationMode::PreDeactivateMode()
{
	FApplicationMode::PreDeactivateMode();

	check(DialogEditor.IsValid());
	TSharedPtr<FDialogBuilderEditor> DialogEditorPtr = DialogEditor.Pin();
	
	DialogEditorPtr->SaveEditedObjectState();
}

void FDialogBuilderEditorApplicationMode::PostActivateMode()
{
	// Reopen any documents that were open when the blueprint was last saved
	check(DialogEditor.IsValid());
	TSharedPtr<FDialogBuilderEditor> DialogEditorPtr = DialogEditor.Pin();
	DialogEditorPtr->RestoreDialogEditor();

	FApplicationMode::PostActivateMode();
}

#undef  LOCTEXT_NAMESPACE

/////////////////////////////////////////////////////
// FDialogBuilderSequencerApplicationMode

#define LOCTEXT_NAMESPACE "BlackboardApplicationMode"

FDialogBuilderSequencerApplicationMode::FDialogBuilderSequencerApplicationMode(TSharedPtr<class FDialogBuilderEditor> InDialogBuilderEditor)
	: FApplicationMode(FDialogBuilderEditor::DialogSequencerMode, FDialogBuilderEditor::GetLocalizedMode)
{
	DialogEditor = InDialogBuilderEditor;
	
	/*DialogBuilderEditorTabFactories.RegisterFactory(MakeShareable(new FBlackboardEditorSummoner(InDialogBuilderEditor)));
	DialogBuilderEditorTabFactories.RegisterFactory(MakeShareable(new FBlackboardDetailsSummoner(InDialogBuilderEditor)));*/

	TabLayout = FTabManager::NewLayout("Standalone_DialogSequencer_Layout_v2")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
#if ENGINE_MAJOR_VERSION < 5
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)->SetHideTabWell(true)
			)
#endif // #if ENGINE_MAJOR_VERSION < 5
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)->SetSizeCoefficient(0.9f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.55f)
					->AddTab("Document", ETabState::ClosedTab)
				)
				->Split
				(
					FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)->SetSizeCoefficient(0.25f)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.55f)
						->AddTab(FDialogBuilderEditorTabs::DialogBuilderPropertyID, ETabState::OpenedTab)
					)
				)
			)
		);

	InDialogBuilderEditor->GetToolbarBuilder()->AddModesToolbar(ToolbarExtender);
}

void FDialogBuilderSequencerApplicationMode::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	check(DialogEditor.IsValid());
	TSharedPtr<FDialogBuilderEditor> DialogEditorPtr = DialogEditor.Pin();
	
	DialogEditorPtr->RegisterToolbarTab(InTabManager.ToSharedRef());

	// Mode-specific setup
	DialogEditorPtr->PushTabFactories(DialogSequencerTabFactories);

	FApplicationMode::RegisterTabFactories(InTabManager);
}

void FDialogBuilderSequencerApplicationMode::PostActivateMode()
{
	// Reopen any documents that were open when the BT was last saved
	check(DialogEditor.IsValid());
	TSharedPtr<FDialogBuilderEditor> DialogEditorPtr = DialogEditor.Pin();

	FApplicationMode::PostActivateMode();
}

#undef LOCTEXT_NAMESPACE
