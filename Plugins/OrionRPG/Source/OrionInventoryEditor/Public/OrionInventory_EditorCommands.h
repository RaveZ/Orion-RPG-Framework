// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "OrionInventory_EditorStyle.h"

class FOrionInventory_EditorCommands : public TCommands<FOrionInventory_EditorCommands>
{
public:

	FOrionInventory_EditorCommands()
		: TCommands<FOrionInventory_EditorCommands>(TEXT("OrionInventoryEditor"), NSLOCTEXT("Contexts", "OrionInventoryEditor", "OrionInventoryEditor Plugin"), NAME_None, FOrionInventory_EditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;

	// New documents
	TSharedPtr< FUICommandInfo > AddNewInventoryGraph;
	
	//Toolbar
	TSharedPtr<FUICommandInfo> NewInventoryDecorator;
	TSharedPtr<FUICommandInfo> NewInventoryEvent;
	TSharedPtr<FUICommandInfo> InventorySetting;
	TSharedPtr<FUICommandInfo> AddInventoryItem;
};