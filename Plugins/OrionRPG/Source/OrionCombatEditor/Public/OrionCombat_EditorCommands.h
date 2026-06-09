// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "OrionCombat_EditorStyle.h"

class FOrionCombat_EditorCommands : public TCommands<FOrionCombat_EditorCommands>
{
public:

	FOrionCombat_EditorCommands()
		: TCommands<FOrionCombat_EditorCommands>(TEXT("OrionCombatEditor"), NSLOCTEXT("Contexts", "OrionCombatEditor", "OrionCombatEditor Plugin"), NAME_None, FOrionCombat_EditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;

};