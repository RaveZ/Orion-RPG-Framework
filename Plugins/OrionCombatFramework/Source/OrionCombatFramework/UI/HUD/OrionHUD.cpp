// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionHUD.h"

#include "Blueprint/UserWidget.h"
#include "OrionCombatFramework/UI/Widget/OrionUserWidget.h"
#include "OrionCombatFramework/UI/WidgetController/OverlayWidgetController.h"
#include "OrionCombatFramework/UI/WidgetController/AttributeMenuWidgetController.h"

UOverlayWidgetController* AOrionHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbacksToDependencies();
		return OverlayWidgetController;
	}
	return OverlayWidgetController;
}

UAttributeMenuWidgetController* AOrionHUD::GetAttributeMenuWidgetController(const FWidgetControllerParams& WCParams)
{
	if (AttributeMenuWidgetController == nullptr)
	{
		AttributeMenuWidgetController = NewObject<UAttributeMenuWidgetController>(this, AttributeMenuWidgetControllerClass);
		AttributeMenuWidgetController->SetWidgetControllerParams(WCParams);
		AttributeMenuWidgetController->BindCallbacksToDependencies();
	}
	return AttributeMenuWidgetController;
}

void AOrionHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("OverlayWidget Class uninitialized, please fill out BP_OrionHUD"));
	checkf(OverlayWidgetControllerClass, TEXT("OverlayWidget Controller Class uninitialized, please fill out BP_OrionHUD"));

	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<UOrionUserWidget>(Widget);

	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
	UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);

	OverlayWidget->SetWidgetController(WidgetController);

	
	WidgetController->BroadcastInitialValues();
	Widget->AddToViewport();
}
