// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Player/OrionPlayerController.h"
#include "OrionCombatFunctionLibrary.h"
#include "AbilitySystem/OrionAbilitySystemComponent.h"

AOrionPlayerController::AOrionPlayerController()
{
}

void AOrionPlayerController::PreProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PreProcessInput(DeltaTime, bGamePaused);
}

void AOrionPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (UOrionAbilitySystemComponent* OrionASC = UOrionCombatFunctionLibrary::GetOrionAbilityComponent(this))
	{
		OrionASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}
