// Fill out your copyright notice in the Description page of Project Settings.


#include "OrionAbilitySystemLibrary.h"

#include "Kismet/GameplayStatics.h"
#include "OrionCombatFramework/Player/OrionPlayerState.h"
#include "OrionCombatFramework/UI/HUD/OrionHUD.h"
#include "OrionCombatFramework/UI/WidgetController/OrionWidgetController.h"

UOverlayWidgetController* UOrionAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (AOrionHUD* OrionHUD = Cast<AOrionHUD>(PC->GetHUD()))
		{
			AOrionPlayerState* PS = PC->GetPlayerState<AOrionPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
			return OrionHUD->GetOverlayWidgetController(WidgetControllerParams);
		}
	}
	return nullptr;
}

UAttributeMenuWidgetController* UOrionAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (AOrionHUD* OrionHUD = Cast<AOrionHUD>(PC->GetHUD()))
		{
			AOrionPlayerState* PS = PC->GetPlayerState<AOrionPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
			return OrionHUD->GetAttributeMenuWidgetController(WidgetControllerParams);
		}
	}
	return nullptr;
}
