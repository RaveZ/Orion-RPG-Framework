// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"

#include "OrionAbilitySystemComponent.generated.h"

class AActor;
class UGameplayAbility;
class UObject;
struct FFrame;


/**
 * UOrionAbilitySystemComponent
 *
 *	Base ability system component class used by this project.
 */
UCLASS(MinimalAPI)
class UOrionAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	UOrionAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

protected:
	// Handles to abilities that had their input pressed this frame.
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	// Handles to abilities that had their input released this frame.
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// Handles to abilities that have their input held.
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

};