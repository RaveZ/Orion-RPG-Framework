// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "DialogBuilder_EditorCommands.h"

#define LOCTEXT_NAMESPACE "FDialog_System_EditorModule"

void FDialogBuilder_EditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Dialog_System_Editor", "Bring up Dialog_System_Editor window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(NewTask, "Graph Settings", "Graph Settings", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(NewDialogCondition, "New Dialog Condition", "New Dialog Condition", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(NewDialogEvent, "New Dialog Event", "New Dialog Event", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(AddNewDialogGraph, "New Dialog Graph", "New Dialog Graph", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control, EKeys::Q));
	UI_COMMAND(DialogSetting, "Dialog Setting", "Open Dialog Setting", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
