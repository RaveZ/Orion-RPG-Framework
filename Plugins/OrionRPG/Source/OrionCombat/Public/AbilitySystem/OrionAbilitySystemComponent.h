// Copyright 2025 Ivan Chandra. All Rights Reserved.UOrionAbilitySystemComponent

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "OrionAbilitySystemComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOrionAbilityCompSignature);

class UOrionAttributeSetBase;
class UOrionAttributeSet;
/**
 * Base gameplay ability system component for Orion combat module.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (OrionCombat), meta = (BlueprintSpawnableComponent))
class ORIONCOMBAT_API UOrionAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	TSubclassOf<UOrionAttributeSet> DefaultAttributeSetClass;
	
	// Attribute sets to grant when we first initialized this component
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttributeSet> StartingAttributes;
	
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "DialogComponent")
	FOrionAbilityCompSignature OnInitAbilityActorInfo;
public:
	UOrionAbilitySystemComponent();
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
	
private:
	// Default Attribute Set
	UPROPERTY()
	TObjectPtr<UOrionAttributeSet> DefaultAttributeSet;
	
protected:
	// Handles to abilities that had their input pressed this frame.
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	// Handles to abilities that had their input released this frame.
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// Handles to abilities that have their input held.
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
	
};