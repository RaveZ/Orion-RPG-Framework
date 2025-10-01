// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "OrionSetting.h"
#include "UObject/ConstructorHelpers.h"
#include "OrionSaveGame.h"

UOrionSetting::UOrionSetting()
{
	//Quest runtime
	
	//UI
	ShowUIBottomBarAction = true;

	//Save
	SaveGameClass = UOrionSaveGame::StaticClass();
	SaveSlotCapacity = 99;
	bSaveLevel = false;
	bSaveDataLayer = true;
}

UOrionSetting::~UOrionSetting()
{
}
