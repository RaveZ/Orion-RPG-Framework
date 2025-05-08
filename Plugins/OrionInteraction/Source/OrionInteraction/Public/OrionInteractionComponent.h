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
	TObjectPtr<class UOrionInteractionTargetComponent> CurrentInteractable;

	/*Index of the interaction list chosen by player*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	int32 InteractionIndex;

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
	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
	void SetInteractionEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
	void StartInteraction();

	UFUNCTION(BlueprintCallable, Category = "QuestComponent")
	void CancelInteraction();
private:
	void UpdateInteractionTrace();
};
