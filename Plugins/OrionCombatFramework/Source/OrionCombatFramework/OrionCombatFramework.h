// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

#define CUSTOM_DEPTH_RED 250

DECLARE_LOG_CATEGORY_EXTERN(OrionLog, Log, All);



class FOrionCombatFrameworkModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
