#pragma once

#include "Components/BoxComponent.h"
#include "OrionInteractionComponent.generated.h"



/**
 * Interaction component
 */
UCLASS( ClassGroup = (Orion), meta = (BlueprintSpawnableComponent, DisplayName = "Interaction Component") )
class UOrionInteractionComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UOrionInteractionComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//We cache the OwningController, we won't cache pawn as this might change
	UPROPERTY(BlueprintReadOnly, Category = "QuestComponent")
	TObjectPtr<APlayerController> OwningController;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	float InteractionTraceDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	float InteractionTraceIntervalSecond;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<class UOrionInteractionTargetComponent> CurrentInteractable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bIsInteractionEnabled;

	FTimerHandle TimerHandle_UpdateInteractionTrace;

public:
	UFUNCTION(BlueprintPure, Category = "QuestComponent")
	virtual APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintPure, Category = "QuestComponent")
	virtual APlayerController* GetOwningController() const;

	UFUNCTION()
	virtual void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
public:
	UFUNCTION(BlueprintCallable, Category = "DO3|PlayerCharacter")
	void SetInteractionEnabled(bool bEnabled);


private:
	void UpdateInteractionTrace();
	void StartInteraction();
	void CancelInteraction();
};
