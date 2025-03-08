// Fill out your copyright notice in the Description page of Project Settings.


#include "OrionHUD.h"

#include "Blueprint/UserWidget.h"
#include "OrionCombatFramework/UI/Widget/OrionUserWidget.h"
#include "OrionCombatFramework/UI/WidgetController/OverlayWidgetController.h"

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
