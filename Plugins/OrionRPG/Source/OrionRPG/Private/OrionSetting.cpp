// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionSetting.h"
#include "UObject/ConstructorHelpers.h"

UOrionSetting::UOrionSetting()
{
	//Quest runtime
	ShowUIBottomBarAction = true;
	bSaveLevel = false;
	bSaveDataLayer = true;
}

UOrionSetting::~UOrionSetting()
{
}
