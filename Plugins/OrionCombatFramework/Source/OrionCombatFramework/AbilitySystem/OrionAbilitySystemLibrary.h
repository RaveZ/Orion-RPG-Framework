// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OrionAbilitySystemLibrary.generated.h"

class UOverlayWidgetController;
class UAttributeMenuWidgetController;
class UAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class ORIONCOMBATFRAMEWORK_API UOrionAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "OrionAbilitySystemLibrary|WidgetController")
	static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "OrionAbilitySystemLibrary|WidgetController")
	static UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContextObject);
	
};
