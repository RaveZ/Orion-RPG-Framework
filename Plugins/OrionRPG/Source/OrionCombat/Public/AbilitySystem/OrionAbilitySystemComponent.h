// Copyright 2025 Ivan Chandra. All Rights Reserved.UOrionAbilitySystemComponent

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"
#include "Attributes/OrionAttributeSet.h"
#include "OrionAbilitySystemComponent.generated.h"


class UOrionAbilitySet;
class UOrionInputConfig;
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_AbilityInputBlocked);

class UOrionGameplayAbility;
class UOrionAttributeSetBase;
class UOrionAttributeSet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOrionAbilityCompSignature);


/**
 * Base gameplay ability system component for Orion combat module.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (OrionCombat), AutoExpandCategories ="AbilityComponent", meta = (BlueprintSpawnableComponent))
class ORIONCOMBAT_API UOrionAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Orion")
	FOrionAbilityCompSignature OnInitAbilityActorInfo;

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Orion")
	TArray<TObjectPtr<UOrionAbilitySet>> AbilitySets;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Orion")
	UOrionInputConfig* InputConfig;
	
public:

	// Delegate when max health changes
	UPROPERTY(BlueprintAssignable, Category = "Orion")
	mutable FOrionAttributeEvent OnMaxHealthChanged;
	
	// Delegate when health changes due to damage/healing, some information may be missing on the client
	UPROPERTY(BlueprintAssignable, Category = "Orion")
	FOrionAttributeEvent OnHealthChanged;
	
	// Delegate to broadcast when the health attribute reaches zero
	UPROPERTY(BlueprintAssignable, Category = "Orion")
	FOrionAttributeEvent OnOutOfHealth;
	
protected:
	// Handles to abilities that had their input pressed this frame.
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	// Handles to abilities that had their input released this frame.
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// Handles to abilities that have their input held.
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
	
public:
	UOrionAbilitySystemComponent();
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;
	
	
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
	
	typedef TFunctionRef<bool(const UOrionGameplayAbility* OrionAbility, FGameplayAbilitySpecHandle Handle)> TShouldCancelAbilityFunc;
	void CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility);

	void CancelInputActivatedAbilities(bool bReplicateCancelAbility);

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);

	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	void ClearAbilityInput();
	
	void TryActivateAbilitiesOnSpawn();
private:
	UFUNCTION()
	virtual void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	
	virtual void InitializePlayerInput(UInputComponent* PlayerInputComponent);
	
};
