// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderEditorToolbar.h"
#include "DialogBuilder_EditorCommands.h"
#include "DialogBuilder_EditorStyle.h"
#include "DialogBuilderEditor.h"

#define LOCTEXT_NAMESPACE "DialogBuilderEditorToolbar"
void FDialogBuilderEditorToolbar::AddDialogSystemToolbar(TSharedPtr<FExtender> Extender)
{
	check(DialogSystemEditor.IsValid());
	TSharedPtr<FDialogBuilderEditor> DialogSystemEditorPtr = DialogSystemEditor.Pin();
	
	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	ToolbarExtender->AddToolBarExtension("Asset", EExtensionHook::After, DialogSystemEditorPtr->GetToolkitCommands(), FToolBarExtensionDelegate::CreateSP(this, &FDialogBuilderEditorToolbar::FillDialogSystemToolbar));
	DialogSystemEditorPtr->AddToolbarExtender(ToolbarExtender);
}

void FDialogBuilderEditorToolbar::FillDialogSystemToolbar(FToolBarBuilder& ToolbarBuilder)
{
	check(DialogSystemEditor.IsValid());
	TSharedPtr<FDialogBuilderEditor> DialogSystemEditorPtr = DialogSystemEditor.Pin();

	ToolbarBuilder.BeginSection("Dialog System");
	{
		const FText NewTaskLabel = LOCTEXT("NewTask_Label", "New Task");
		const FText NewTaskTooltip = LOCTEXT("NewTask_ToolTip", "Create a new task node Blueprint from a base class");
		const FSlateIcon NewTaskIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "BTEditor.Graph.NewTask");

		
		ToolbarBuilder.AddToolBarButton(FDialogBuilder_EditorCommands::Get().NewTask,
			NAME_None,
			NewTaskLabel,
			NewTaskTooltip,
			NewTaskIcon
		);
	}
	ToolbarBuilder.EndSection();

	ToolbarBuilder.BeginSection("Dialog System");
	{
		const FText NewConditionLabel = LOCTEXT("NewDialogCondition_Label", "New Dialog Condition");
		const FText NewConditionTooltip = LOCTEXT("NewDialogCondition_ToolTip", "Create a new Dialog Condition Blueprint from a base class");
		const FSlateIcon NewConditionIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "BTEditor.Graph.BTNode.Decorator.Conditional.Icon");


		ToolbarBuilder.AddToolBarButton(FDialogBuilder_EditorCommands::Get().NewDialogCondition,
			NAME_None,
			NewConditionLabel,
			NewConditionTooltip,
			NewConditionIcon
		);
	}
	ToolbarBuilder.EndSection();

	ToolbarBuilder.BeginSection("Dialog System");
	{
		const FText NewEventLabel = LOCTEXT("NewDialogEvent_Label", "New Dialog Event");
		const FText NewEventTooltip = LOCTEXT("NewDialogEvent_ToolTip", "Create a new Dialog Event Blueprint from a base class");
		const FSlateIcon NewEventIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.CustomEvent_16x");


		ToolbarBuilder.AddToolBarButton(FDialogBuilder_EditorCommands::Get().NewDialogEvent,
			NAME_None,
			NewEventLabel,
			NewEventTooltip,
			NewEventIcon
		);
	}
	ToolbarBuilder.EndSection();

	ToolbarBuilder.BeginSection("Dialog System");
	{
		const FText DialogSettingLabel = LOCTEXT("DialogSetting_Label", "Dialog Default Info");
		const FText DialogSettingTooltip = LOCTEXT("DialogSetting_ToolTip", "Access Dialog Default Info");
		const FSlateIcon DialogSettingIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ProjectSettings.TabIcon");


		ToolbarBuilder.AddToolBarButton(FDialogBuilder_EditorCommands::Get().DialogSetting,
			NAME_None,
			DialogSettingLabel,
			DialogSettingTooltip,
			DialogSettingIcon
		);
	}
	ToolbarBuilder.EndSection();

	
}



#undef LOCTEXT_NAMESPACE
