// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionWidgetController.h"

void UOrionWidgetController::SetWidgetControllerParams(const FWidgetControllerParams WCParams)
{
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	AttributeSet = WCParams.AttributeSet;
}

void UOrionWidgetController::BroadcastInitialValues()
{
	
}

void UOrionWidgetController::BindCallbacksToDependencies()
{
}
