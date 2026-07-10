// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionCombatFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/OrionAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/OrionAttributeSetBase.h"
#include "Animation/OrionMovementLayerComponent.h"
#include "GameFramework/Character.h"
#include "TargetingSystem/OrionTargetingSystemComponent.h"
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

	//Dialog comp may be on the pawn or player state
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

class UOrionTargetingSystemComponent* UOrionCombatFunctionLibrary::GetTargetingSystemComponent(
	const UObject* WorldContextObject)
{
	return GetTargetingSystemComponentFromTarget(UGameplayStatics::GetPlayerController(WorldContextObject, 0));
}


class UOrionTargetingSystemComponent* UOrionCombatFunctionLibrary::GetTargetingSystemComponentFromTarget(AActor* Target)
{
	
	if (!Target)
	{
		return nullptr;
	}

	if (UOrionTargetingSystemComponent* TargetingComp = Target->FindComponentByClass<UOrionTargetingSystemComponent>())
	{
		return TargetingComp;
	}

	//targeting comp may be on the controllers pawn or pawns controller
	if (APlayerController* OwningController = Cast<APlayerController>(Target))
	{
		if (OwningController->GetPawn())
		{
			return OwningController->GetPawn()->FindComponentByClass<UOrionTargetingSystemComponent>();
		}
	}

	if (APawn* OwningPawn = Cast<APawn>(Target))
	{
		if (OwningPawn->GetController())
		{
			return OwningPawn->GetController()->FindComponentByClass<UOrionTargetingSystemComponent>();
		}
	}

	return nullptr;
}

class UOrionMovementLayerComponent* UOrionCombatFunctionLibrary::GetMovementLayerComponent(ACharacter* Character)
{
	if (Character)
	{
		return Character->FindComponentByClass<UOrionMovementLayerComponent>();
	}
	return nullptr;
}

bool UOrionCombatFunctionLibrary::IsAlive(AActor* Target)
{
	
	if (!Target) return false;
    
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!ASC) return false;
    
	UOrionAttributeSetBase* AttributeSet = const_cast<UOrionAttributeSetBase*>(ASC->GetSet<UOrionAttributeSetBase>());
	if (!AttributeSet) return false;
    
	return AttributeSet->IsAlive();
}

void UOrionCombatFunctionLibrary::SetMovementOverlay(ACharacter* Character, FGameplayTag Tag)
{
	UOrionMovementLayerComponent* MovementLayer = Character ? GetMovementLayerComponent(Character) : nullptr;
	if (MovementLayer)
	{
		MovementLayer->SetOverlay(Tag);
	}
}
