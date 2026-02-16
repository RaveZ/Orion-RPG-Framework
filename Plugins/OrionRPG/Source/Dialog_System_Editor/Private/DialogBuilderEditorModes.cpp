// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "DialogBuilderEditorModes.h"

#include "DialogBuilderEditor.h"
#include "DialogBuilderEditorToolbar.h"
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

	DialogBuilderEditorTabFactories.RegisterFactory(MakeShareable(new FBehaviorTreeDetailsSummoner(InDialogBuilderEditor)));
	DialogBuilderEditorTabFactories.RegisterFactory(MakeShareable(new FBehaviorTreeSearchSummoner(InDialogBuilderEditor)));
	DialogBuilderEditorTabFactories.RegisterFactory(MakeShareable(new FBlackboardSummoner(InDialogBuilderEditor)));

	TabLayout = FTabManager::NewLayout( "Standalone_BehaviorTree_Layout_v2" )
	->AddArea
	(
		FTabManager::NewPrimaryArea() ->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewSplitter() ->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.7f)
				->AddTab(FDialogBuilderEditorTabs::GraphEditorID, ETabState::ClosedTab)
			)
			->Split
			(
				FTabManager::NewSplitter() ->SetOrientation(Orient_Vertical)
				->SetSizeCoefficient(0.3f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.6f)
					->AddTab(FDialogBuilderEditorTabs::GraphDetailsID, ETabState::OpenedTab)
					->AddTab(FDialogBuilderEditorTabs::SearchID, ETabState::ClosedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.4f)
					->AddTab(FDialogBuilderEditorTabs::BlackboardID, ETabState::OpenedTab)
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
	TSharedPtr<FDialogBuilderEditor> BehaviorTreeEditorPtr = DialogEditor.Pin();
	
	BehaviorTreeEditorPtr->RegisterToolbarTab(InTabManager.ToSharedRef());

	// Mode-specific setup
	BehaviorTreeEditorPtr->PushTabFactories(DialogBuilderEditorTabFactories);

	FApplicationMode::RegisterTabFactories(InTabManager);
}

void FDialogBuilderEditorApplicationMode::PreDeactivateMode()
{
	FApplicationMode::PreDeactivateMode();

	check(DialogEditor.IsValid());
	TSharedPtr<FDialogBuilderEditor> BehaviorTreeEditorPtr = DialogEditor.Pin();
	
	BehaviorTreeEditorPtr->SaveEditedObjectState();
}

void FDialogBuilderEditorApplicationMode::PostActivateMode()
{
	// Reopen any documents that were open when the blueprint was last saved
	check(BehaviorTreeEditor.IsValid());
	TSharedPtr<FDialogBuilderEditor> BehaviorTreeEditorPtr = DialogEditor.Pin();
	BehaviorTreeEditorPtr->RestoreBehaviorTree();

	FApplicationMode::PostActivateMode();
}

#undef  LOCTEXT_NAMESPACE

/////////////////////////////////////////////////////
// FDialogBuilderSequencerApplicationMode

#define LOCTEXT_NAMESPACE "BlackboardApplicationMode"

FDialogBuilderSequencerApplicationMode::FDialogBuilderSequencerApplicationMode(TSharedPtr<class FDialogBuilderEditor> InDialogBuilderEditor)
	: FApplicationMode(FDialogBuilderEditor::BlackboardMode, FDialogBuilderEditor::GetLocalizedMode)
{
	DialogEditor = InDialogBuilderEditor;
	
	DialogBuilderEditorTabFactories.RegisterFactory(MakeShareable(new FBlackboardEditorSummoner(InDialogBuilderEditor)));
	DialogBuilderEditorTabFactories.RegisterFactory(MakeShareable(new FBlackboardDetailsSummoner(InDialogBuilderEditor)));

	TabLayout = FTabManager::NewLayout( "Standalone_BlackboardEditor_Layout_v2" )
	->AddArea
	(
		FTabManager::NewPrimaryArea() ->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewSplitter() ->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewStack()
				->AddTab(FDialogBuilderEditorTabs::BlackboardEditorID, ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewStack()
				->AddTab(FDialogBuilderEditorTabs::BlackboardDetailsID, ETabState::OpenedTab)
			)
		)
	);

	InDialogBuilderEditor->GetToolbarBuilder()->AddModesToolbar(ToolbarExtender);
}

void FDialogBuilderSequencerApplicationMode::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	check(DialogEditor.IsValid());
	TSharedPtr<FDialogBuilderEditor> BehaviorTreeEditorPtr = DialogEditor.Pin();
	
	BehaviorTreeEditorPtr->RegisterToolbarTab(InTabManager.ToSharedRef());

	// Mode-specific setup
	BehaviorTreeEditorPtr->PushTabFactories(BlackboardTabFactories);

	FApplicationMode::RegisterTabFactories(InTabManager);
}

void FDialogBuilderSequencerApplicationMode::PostActivateMode()
{
	// Reopen any documents that were open when the BT was last saved
	check(DialogEditor.IsValid());
	TSharedPtr<FDialogBuilderEditor> BehaviorTreeEditorPtr = DialogEditor.Pin();

	FApplicationMode::PostActivateMode();
}

#undef LOCTEXT_NAMESPACE
