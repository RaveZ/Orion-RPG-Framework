// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "Components/BoxComponent.h"
#include "OrionInteractionComponent.generated.h"


UENUM(BlueprintType)
enum class EOrionInteractionType : uint8
{
	/**Interaction trace from camera*/
	E_InteractionCamera		UMETA(DisplayName = "Camera"),
	/**Interaction trace from pawn*/
	E_InteractionPawn		UMETA(DisplayName = "Pawn"),
};

class UOrionInteractionTargetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrionInteractionSignature, UOrionInteractionTargetComponent*, InteractionTarget);

/**
 * Interaction component
 */
UCLASS(meta = (BlueprintSpawnableComponent), Blueprintable, HideDropdown, ClassGroup = "OrionRPG", DisplayName = "Orion Interaction")
class UOrionInteractionComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UOrionInteractionComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//We cache the OwningController, we won't cache pawn as this might change
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<APlayerController> OwningController;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
    TEnumAsByte<ECollisionChannel> InteractionTraceChannel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	EOrionInteractionType InteractionType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction", meta = (DisplayName = "Debug Trace?"))
	bool bDebugTrace;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction", meta = (EditCondition = "InteractionType == EOrionInteractionType::E_InteractionPawn", HideEditConditionToggle = true, EditConditionHides = true))  
    FVector InteractionOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction", meta = (EditCondition = "InteractionType == EOrionInteractionType::E_InteractionPawn", HideEditConditionToggle = true, EditConditionHides = true))
	float InteractionRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction", meta = (EditCondition = "InteractionType == EOrionInteractionType::E_InteractionCamera", HideEditConditionToggle = true, EditConditionHides = true))
	float InteractionTraceDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	float InteractionTraceIntervalSecond;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TArray<FHitResult> InteractionhitResults;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<class UOrionInteractionTargetComponent> CurrentInteractionTarget;

	/*Index of the interaction list chosen by player*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	int32 InteractionIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bIsInteractionEnabled;

	FTimerHandle TimerHandle_UpdateInteractionTrace;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOrionInteractionSignature OnInteractionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOrionInteractionSignature OnInteractionCanceled;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOrionInteractionSignature OnInteractionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOrionInteractionSignature OnInteractionUpdated;
public:
	UFUNCTION(BlueprintPure, Category = "Interaction")
	virtual APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	virtual APlayerController* GetOwningController() const;

	UFUNCTION()
	virtual void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
public:
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractionEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void StartInteraction();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void CancelInteraction();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void InteractionEnded(UOrionInteractionTargetComponent* Component, bool bCompleted);
private:
	void UpdateInteractionTrace();
};
