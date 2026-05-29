// Copyright 2025 Ivan Chandra. All Rights Reserved.UOrionAbilitySystemComponent

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "OrionAbilitySystemComponent.generated.h"

/**
 * Base gameplay ability system component for Orion combat module.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (OrionCombat), meta = (BlueprintSpawnableComponent))
class ORIONCOMBAT_API UOrionAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UOrionAbilitySystemComponent();

protected:
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
};