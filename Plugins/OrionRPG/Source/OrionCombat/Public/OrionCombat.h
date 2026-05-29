// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"




class FOrionCombatModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
