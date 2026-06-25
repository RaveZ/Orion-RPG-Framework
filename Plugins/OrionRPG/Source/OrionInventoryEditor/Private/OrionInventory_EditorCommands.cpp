// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "OrionInventory_EditorCommands.h"

#define LOCTEXT_NAMESPACE "FInventory_System_EditorModule"

void FOrionInventory_EditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "OrionInventoryEditor", "Bring up Inventory_System_Editor window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(NewInventoryFragment, "New Item Fragment", "New Item Fragment", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(AddNewInventoryGraph, "New Inventory Graph", "New Inventory Graph", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control, EKeys::Q));
	UI_COMMAND(InventorySetting, "Inventory Setting", "Open Inventory Setting", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(AddInventoryItem, "Add Inventory Item", "Add Inventory Item", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
