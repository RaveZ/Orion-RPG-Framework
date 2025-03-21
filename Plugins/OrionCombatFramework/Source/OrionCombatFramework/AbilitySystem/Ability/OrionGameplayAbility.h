// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "OrionGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class ORIONCOMBATFRAMEWORK_API UOrionGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FGameplayTag StartupInputTag;
};
