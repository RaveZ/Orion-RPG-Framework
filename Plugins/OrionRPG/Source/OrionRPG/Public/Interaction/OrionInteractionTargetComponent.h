// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "Components/BoxComponent.h"
#include "Event/OrionEvent.h"
#include "Decorator/OrionDecorator.h"
#include "OrionInteractionTargetComponent.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOrionInteractionTargetSignature, AActor*, Interactor, UOrionInteractionTargetComponent*, Component);



UENUM(BlueprintType)
enum class EOrionInteractionMode : uint8
{
	None	UMETA(DisplayName = "None"),
	Tap		UMETA(DisplayName = "Tap"),
	Hold	UMETA(DisplayName = "Hold")
};



/**
 * Interaction component
 */
UCLASS(meta = (BlueprintSpawnableComponent), Blueprintable, HideDropdown, ClassGroup = "OrionRPG", DisplayName = "Orion Interaction Target")
class UOrionInteractionTargetComponent : public USceneComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Interactable")
	FOrionInteractionTargetSignature OnFocused;

	UPROPERTY(BlueprintAssignable, Category = "Interactable")
	FOrionInteractionTargetSignature OnStarted;

	UPROPERTY(BlueprintAssignable, Category = "Interactable")
	FOrionInteractionTargetSignature OnProcessed;

	UPROPERTY(BlueprintAssignable, Category = "Interactable")
	FOrionInteractionTargetSignature OnCanceled;

	UPROPERTY(BlueprintAssignable, Category = "Interactable")
	FOrionInteractionTargetSignature OnCompleted;

	/**
	* Tag for this component.
	* Also Add this tag to the owning actor tag.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	EOrionInteractionMode Mode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable", meta = (EditCondition = "Mode == EOrionInteractionMode::Hold", HideEditConditionToggle, EditConditionHides))
	float HoldDuration;

	float CurrentHoldTime;

	/**
	* Interact conditions that must be met before interaction can occur.
	*/
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "Interactable", DisplayName = "Interact Conditions")
	TArray<TObjectPtr<UOrionDecorator>> InteractConditions;

	/**
	* When interaction conditions not met, it will be counted as blocked interaction.
	* you can add notification and the reason here.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable")
	FText InteractionBlockedReason;

	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "Interactable", DisplayName = "Interaction Completed Events")
	TArray<TObjectPtr<UOrionEvent>> Events;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable")
	TObjectPtr<AActor> CurrentInteractor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable")
	float CurrentProgress;

	//Ensure Each interaction has cooldown in between and avoid interacting the same object twice in one frame
	UPROPERTY(Transient)
	bool bNativeCanInteract;

private:
	FTimerHandle TimerHandle_UpdateProgressValue;

	FTimerHandle TimerHandle_ResetInteraction;

public:
	UOrionInteractionTargetComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "Interactable")
	bool CanInteract() const;

	void Focused(AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category = "Interactable")
	void StartInteraction(AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category = "Interactable")
	void EndInteraction(bool bCompleted);

	UFUNCTION(BlueprintPure, Category = "Interactable")
	bool IsInteractionInProgress() const;

	UFUNCTION(BlueprintCallable, Category = "Interactable")
	void UpdateInteractionProgress(float Progress);

	UFUNCTION(Category = "Interactable")
	bool DecoratorConditionMet() const;
private:
	void UpdateProgressValue();


// ================================ HUD ================================ //
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD")
	FText Description;

};
