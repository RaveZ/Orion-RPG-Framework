// Copyright 2025 Ivan Chandra. All Rights Reserved.UOrionAbilitySystemComponent

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "OrionAbilitySystemComponent.generated.h"

class UOrionDefaultAttributeSet;
class UOrionAttributeSet;
/**
 * Base gameplay ability system component for Orion combat module.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (OrionCombat), meta = (BlueprintSpawnableComponent))
class ORIONCOMBAT_API UOrionAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UOrionAbilitySystemComponent();
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
	
private:
	// Default Attribute Set
	UPROPERTY()
	TObjectPtr<UOrionDefaultAttributeSet> DefaultAttributeSet;
};