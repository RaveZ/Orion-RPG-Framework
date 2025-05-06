#pragma once

#include "Components/BoxComponent.h"
#include "OrionInteractionTargetComponent.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOrionInteractionSignature, AActor*, Interactor, UOrionInteractionTargetComponent*, Component);



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
UCLASS( ClassGroup = (Orion), meta = (BlueprintSpawnableComponent) )
class UOrionInteractionTargetComponent : public UBoxComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Interactable")
	FOrionInteractionSignature OnFocused;

	UPROPERTY(BlueprintAssignable, Category = "Interactable")
	FOrionInteractionSignature OnStarted;

	UPROPERTY(BlueprintAssignable, Category = "Interactable")
	FOrionInteractionSignature OnProcessed;

	UPROPERTY(BlueprintAssignable, Category = "Interactable")
	FOrionInteractionSignature OnCanceled;

	UPROPERTY(BlueprintAssignable, Category = "Interactable")
	FOrionInteractionSignature OnCompleted;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
	EOrionInteractionMode Mode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable", meta = (EditCondition = "Mode == EOrionInteractionMode::Hold"))
	float HoldDuration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable")
	TObjectPtr<AActor> CurrentInteractor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable")
	float CurrentProgress;

private:
	FTimerHandle TimerHandle_UpdateProgressValue;


public:
	UOrionInteractionTargetComponent();

	UFUNCTION(BlueprintPure, Category = "DO3|Interactable")
	bool CanInteract() const;

	void Focused(AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category = "DO3|Interactable")
	void StartInteraction(AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category = "DO3|Interactable")
	void EndInteraction(bool bCompleted);

	UFUNCTION(BlueprintPure, Category = "DO3|Interactable")
	bool IsInteractionInProgress() const;

	UFUNCTION(BlueprintCallable, Category = "DO3|Interactable")
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
