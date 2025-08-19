#pragma once

#include "Components/BoxComponent.h"
#include "Event/OrionEvent.h"
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable")
	TObjectPtr<AActor> CurrentInteractor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable")
	float CurrentProgress;

	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "Interactable", DisplayName="Interaction Completed Events")
	TArray<TObjectPtr<UOrionEvent>> Events;
private:
	FTimerHandle TimerHandle_UpdateProgressValue;


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
