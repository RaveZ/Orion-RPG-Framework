// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OrionCombatFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class ORIONCOMBAT_API UOrionCombatFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	
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
	* Grab the Targeting Ability System component from the local player controller.
	*
	* @return The Targeting System Component
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "OrionLibrary|Targeting", meta = (WorldContext = "WorldContextObject"))
	static class UOrionTargetingSystemComponent* GetTargetingSystemComponent(const UObject* WorldContextObject);
	
	/**
	* Find the Targeting Ability System component from the supplied target object.
	*
	* @return The Targeting System Component
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "OrionLibrary|Ability", meta = (DefaultToSelf = "Target"))
	static class UOrionTargetingSystemComponent* GetTargetingSystemComponentFromTarget(AActor* Target);
	
	
};
