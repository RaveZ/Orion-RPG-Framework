// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderEditorToolbar.h"
#include "DialogBuilder_EditorCommands.h"
#include "DialogBuilder_EditorStyle.h"
#include "DialogBuilderEditor.h"

#define LOCTEXT_NAMESPACE "QuestBuilderEditorToolbar"
void FDialogBuilderEditorToolbar::AddQuestSystemToolbar(TSharedPtr<FExtender> Extender)
{
	check(QuestSystemEditor.IsValid());
	TSharedPtr<FDialogBuilderEditor> QuestSystemEditorPtr = QuestSystemEditor.Pin();
	
	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	ToolbarExtender->AddToolBarExtension("Asset", EExtensionHook::After, QuestSystemEditorPtr->GetToolkitCommands(), FToolBarExtensionDelegate::CreateSP(this, &FDialogBuilderEditorToolbar::FillQuestSystemToolbar));
	QuestSystemEditorPtr->AddToolbarExtender(ToolbarExtender);
}

void FDialogBuilderEditorToolbar::FillQuestSystemToolbar(FToolBarBuilder& ToolbarBuilder)
{
	check(QuestSystemEditor.IsValid());
	TSharedPtr<FDialogBuilderEditor> QuestSystemEditorPtr = QuestSystemEditor.Pin();

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
		const FText NewConditionLabel = LOCTEXT("NewQuestCondition_Label", "New Dialog Condition");
		const FText NewConditionTooltip = LOCTEXT("NewQuestCondition_ToolTip", "Create a new Dialog Condition Blueprint from a base class");
		const FSlateIcon NewConditionIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "BTEditor.Graph.BTNode.Decorator.Conditional.Icon");


		ToolbarBuilder.AddToolBarButton(FDialogBuilder_EditorCommands::Get().NewQuestCondition,
			NAME_None,
			NewConditionLabel,
			NewConditionTooltip,
			NewConditionIcon
		);
	}
	ToolbarBuilder.EndSection();

	ToolbarBuilder.BeginSection("Dialog System");
	{
		const FText NewEventLabel = LOCTEXT("NewQuestEvent_Label", "New Dialog Event");
		const FText NewEventTooltip = LOCTEXT("NewQuestEvent_ToolTip", "Create a new Dialog Event Blueprint from a base class");
		const FSlateIcon NewEventIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.CustomEvent_16x");


		ToolbarBuilder.AddToolBarButton(FDialogBuilder_EditorCommands::Get().NewQuestEvent,
			NAME_None,
			NewEventLabel,
			NewEventTooltip,
			NewEventIcon
		);
	}
	ToolbarBuilder.EndSection();

	ToolbarBuilder.BeginSection("Dialog System");
	{
		const FText QuestSettingLabel = LOCTEXT("QuestSetting_Label", "Dialog Default Info");
		const FText QuestSettingTooltip = LOCTEXT("QuestSetting_ToolTip", "Access Dialog Default Info");
		const FSlateIcon QuestSettingIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ProjectSettings.TabIcon");


		ToolbarBuilder.AddToolBarButton(FDialogBuilder_EditorCommands::Get().QuestSetting,
			NAME_None,
			QuestSettingLabel,
			QuestSettingTooltip,
			QuestSettingIcon
		);
	}
	ToolbarBuilder.EndSection();

	
}



#undef LOCTEXT_NAMESPACE
