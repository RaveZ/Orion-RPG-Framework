// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderEditorToolbar.h"
#include "QuestBuilder_EditorCommands.h"
#include "QuestBuilder_EditorStyle.h"
#include "QuestBuilderEditor.h"

#define LOCTEXT_NAMESPACE "QuestBuilderEditorToolbar"
void FQuestBuilderEditorToolbar::AddQuestSystemToolbar(TSharedPtr<FExtender> Extender)
{
	check(QuestSystemEditor.IsValid());
	TSharedPtr<FQuestBuilderEditor> QuestSystemEditorPtr = QuestSystemEditor.Pin();
	
	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	ToolbarExtender->AddToolBarExtension("Asset", EExtensionHook::After, QuestSystemEditorPtr->GetToolkitCommands(), FToolBarExtensionDelegate::CreateSP(this, &FQuestBuilderEditorToolbar::FillQuestSystemToolbar));
	QuestSystemEditorPtr->AddToolbarExtender(ToolbarExtender);
}

void FQuestBuilderEditorToolbar::FillQuestSystemToolbar(FToolBarBuilder& ToolbarBuilder)
{
	check(QuestSystemEditor.IsValid());
	TSharedPtr<FQuestBuilderEditor> QuestSystemEditorPtr = QuestSystemEditor.Pin();

	ToolbarBuilder.BeginSection("Quest System");
	{
		const FText NewTaskLabel = LOCTEXT("NewTask_Label", "New Task");
		const FText NewTaskTooltip = LOCTEXT("NewTask_ToolTip", "Create a new task node Blueprint from a base class");
		const FSlateIcon NewTaskIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "BTEditor.Graph.NewTask");

		
		ToolbarBuilder.AddToolBarButton(FQuestBuilder_EditorCommands::Get().NewTask,
			NAME_None,
			NewTaskLabel,
			NewTaskTooltip,
			NewTaskIcon
		);
	}
	ToolbarBuilder.EndSection();

	ToolbarBuilder.BeginSection("Quest System");
	{
		const FText NewConditionLabel = LOCTEXT("NewQuestCondition_Label", "New Quest Condition");
		const FText NewConditionTooltip = LOCTEXT("NewQuestCondition_ToolTip", "Create a new Quest Condition Blueprint from a base class");
		const FSlateIcon NewConditionIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "BTEditor.Graph.BTNode.Decorator.Conditional.Icon");


		ToolbarBuilder.AddToolBarButton(FQuestBuilder_EditorCommands::Get().NewQuestCondition,
			NAME_None,
			NewConditionLabel,
			NewConditionTooltip,
			NewConditionIcon
		);
	}
	ToolbarBuilder.EndSection();

	ToolbarBuilder.BeginSection("Quest System");
	{
		const FText NewEventLabel = LOCTEXT("NewQuestEvent_Label", "New Quest Event");
		const FText NewEventTooltip = LOCTEXT("NewQuestEvent_ToolTip", "Create a new Quest Event Blueprint from a base class");
		const FSlateIcon NewEventIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.CustomEvent_16x");


		ToolbarBuilder.AddToolBarButton(FQuestBuilder_EditorCommands::Get().NewQuestEvent,
			NAME_None,
			NewEventLabel,
			NewEventTooltip,
			NewEventIcon
		);
	}
	ToolbarBuilder.EndSection();

	ToolbarBuilder.BeginSection("Quest System");
	{
		const FText QuestSettingLabel = LOCTEXT("QuestSetting_Label", "Quest Default Info");
		const FText QuestSettingTooltip = LOCTEXT("QuestSetting_ToolTip", "Access Quest Default Info");
		const FSlateIcon QuestSettingIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ProjectSettings.TabIcon");


		ToolbarBuilder.AddToolBarButton(FQuestBuilder_EditorCommands::Get().QuestSetting,
			NAME_None,
			QuestSettingLabel,
			QuestSettingTooltip,
			QuestSettingIcon
		);
	}
	ToolbarBuilder.EndSection();

	
}



#undef LOCTEXT_NAMESPACE
