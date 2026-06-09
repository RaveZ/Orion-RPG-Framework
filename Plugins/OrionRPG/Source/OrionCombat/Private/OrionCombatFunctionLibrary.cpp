// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionCombatFunctionLibrary.h"

#include "AbilitySystem/OrionAbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

class UOrionAbilitySystemComponent* UOrionCombatFunctionLibrary::GetOrionAbilityComponent(const UObject* WorldContextObject)
{
	return GetOrionAbilityComponentFromTarget(UGameplayStatics::GetPlayerPawn(WorldContextObject, 0));
	
}

class UOrionAbilitySystemComponent* UOrionCombatFunctionLibrary::GetOrionAbilityComponentFromTarget(AActor* Target)
{
	if (!Target)
	{
		return nullptr;
	}
	if (UOrionAbilitySystemComponent* AbilityComp = Target->FindComponentByClass<UOrionAbilitySystemComponent>())
	{
		return AbilityComp;
	}

	//Dialog comp may be on the controllers pawn or pawns controller
	if (APlayerState* OwningPlayerState = Cast<APlayerState>(Target))
	{
		if (OwningPlayerState->GetPawn())
		{
			return OwningPlayerState->GetPawn()->FindComponentByClass<UOrionAbilitySystemComponent>();
		}
	}

	if (APawn* OwningPawn = Cast<APawn>(Target))
	{
		if (OwningPawn->GetPlayerState())
		{
			return OwningPawn->GetPlayerState()->FindComponentByClass<UOrionAbilitySystemComponent>();
		}
	}

	return nullptr;
}
