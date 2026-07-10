// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OrionCombatFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class ORIONCOMBAT_API UOrionCombatFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/**
	* Grab the Orion Ability System component from the local pawn or player state, whichever it exists on.
	*
	* @return The Ability System Component
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "OrionLibrary|Ability", meta = (WorldContext = "WorldContextObject"))
	static class UOrionAbilitySystemComponent* GetOrionAbilityComponent(const UObject* WorldContextObject);
	
	
	/**
	* Find the Orion Ability System component from the supplied target object.
	*
	* @return The Ability System Component
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "OrionLibrary|Ability", meta = (DefaultToSelf = "Target"))
	static class UOrionAbilitySystemComponent* GetOrionAbilityComponentFromTarget(AActor* Target);
	
	/**
	* Check if the actor is alive
	*
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "OrionLibrary|Ability")
	static bool IsAlive(AActor* Target);
	
	
	/**
	* Grab the Targeting System component from the local player controller.
	*
	* @return The Targeting System Component
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "OrionLibrary|Targeting", meta = (WorldContext = "WorldContextObject"))
	static class UOrionTargetingSystemComponent* GetTargetingSystemComponent(const UObject* WorldContextObject);
	
	/**
	* Find the Targeting System component from the supplied target object.
	*
	* @return The Targeting System Component
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "OrionLibrary|Targeting", meta = (DefaultToSelf = "Target"))
	static class UOrionTargetingSystemComponent* GetTargetingSystemComponentFromTarget(AActor* Target);
	
	/**
	* Find the Targeting System component from the supplied target object.
	*
	* @return The Targeting System Component
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "OrionLibrary|Movement")
	static class UOrionMovementLayerComponent* GetMovementLayerComponent(ACharacter* Character);
	
	/**
	* Set Movement Overlay
	*
	*/
	UFUNCTION(BlueprintCallable, Category = "OrionLibrary|Movement")
	static void SetMovementOverlay(ACharacter* Character, UPARAM(meta = (Categories = "MovementLayer"))FGameplayTag Tag);
	
	
};
