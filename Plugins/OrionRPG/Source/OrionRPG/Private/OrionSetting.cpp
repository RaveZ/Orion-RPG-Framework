// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "OrionSetting.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "OrionSaveGame.h"

UOrionSetting::UOrionSetting()
{
	//Quest runtime
	
	//UI
	ShowUIBottomBarAction = true;
	auto OverlayWidgetFinder = ConstructorHelpers::FClassFinder<UUserWidget>(TEXT("WidgetBlueprint'/OrionRPG/Widgets/WBP_Overlay.WBP_Overlay_C'"));
	if (OverlayWidgetFinder.Succeeded())
	{
		DefaultOverlayWidget = OverlayWidgetFinder.Class;
	}

	//Save
	SaveGameClass = UOrionSaveGame::StaticClass();
	SaveSlotCapacity = 99;
	bSaveLevel = false;
	bSaveDataLayer = true;

	//Input
	auto InputDTFinder = ConstructorHelpers::FObjectFinder<UDataTable>(TEXT("DataTable'/OrionRPG/CommonUI/Data/DT_OrionInputActions.DT_OrionInputActions'"));
	if (InputDTFinder.Succeeded())
	{
		InteractInput.DataTable = InputDTFinder.Object;
		InteractInput.RowName = FName("Interact");
	}
}

UOrionSetting::~UOrionSetting()
{
}
