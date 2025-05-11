// Copyright 2024 Ivan Chandra. All Rights Reserved.

#include "DialogBuilder_EditorCommands.h"

#define LOCTEXT_NAMESPACE "FDialog_System_EditorModule"

void FDialogBuilder_EditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Dialog_System_Editor", "Bring up Dialog_System_Editor window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(NewTask, "Graph Settings", "Graph Settings", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(NewQuestCondition, "New Dialog Condition", "New Dialog Condition", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(NewQuestEvent, "New Dialog Event", "New Dialog Event", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(AddNewQuestGraph, "New Dialog Graph", "New Dialog Graph", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control, EKeys::Q));
	UI_COMMAND(QuestSetting, "Dialog Setting", "Open Dialog Setting", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
