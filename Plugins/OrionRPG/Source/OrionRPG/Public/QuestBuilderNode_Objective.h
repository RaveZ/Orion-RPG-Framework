// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderNode_Objective.generated.h"

class UQuestObjective;


UENUM(BlueprintType)
enum class EObjectiveState : uint8
{
	E_Active		UMETA(DisplayName = "ACTIVE"),
	E_Completed		UMETA(DisplayName = "COMPLETED"),
	E_Failed			UMETA(DisplayName = "FAILED"),
};

UCLASS(Abstract, Blueprintable, EditInlineNew, AutoExpandCategories = ("Default"))
class ORIONRPG_API UQuestBuilderNode_Objective : public UQuestBuilderNode
{
	GENERATED_BODY()

public:
	UQuestBuilderNode_Objective();
	/**List of objective for your objective*/
	UPROPERTY(Instanced, BlueprintReadOnly, Category = "Objective", meta = (FullyExpand = true))
		TArray<TObjectPtr<UQuestTask>> Tasks;

	/**mark the objective as optional and skip this objective if the other non - optional objective has been completed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	bool bOptional;

	/**Hide the objective from widget*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	bool bHidden;

	/**This objective use amount required to complete objective?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective", meta = (DisplayName = "UseAmount"))
	bool bUseAmount;

	/**Amount required for your objective*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective", meta = (EditCondition = "bUseAmount == true", EditConditionHides))
	int RequiredAmount;

	/**Is Objective Tickable?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective", meta = (DisplayName = "ObjectiveTickable"))
	bool bIsObjectiveTickable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective", meta = (EditCondition = "bIsObjectiveTickable == true", EditConditionHides))
	float TickInterval;

	UPROPERTY(BlueprintReadWrite, Category = "Objective")
	EObjectiveState ObjectiveState = EObjectiveState::E_Active;

	UPROPERTY(BlueprintReadWrite, Category = "Objective")
	int CurrentAmount = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Objective")
	float DistanceToTarget;

	FTimerHandle TimerHandle_TickObjective;

public:
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Begin Objective", Category = "Objective")
	void K2_BeginObjective(AController* OwnerController, APawn* ControlledPawn);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Objective Completed", Category = "Objective")
	void K2_ObjectiveCompleted(AController* OwnerController, APawn* ControlledPawn);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Objective Failed", Category = "Objective")
	void K2_ObjectiveFailed(AController* OwnerController, APawn* ControlledPawn);

	UFUNCTION(BlueprintCallable, Category = "Objective")
	virtual void BeginObjective();

	UFUNCTION(BlueprintCallable, Category = "Objective")
	virtual bool CompleteObjective();

	UFUNCTION(BlueprintCallable, Category = "Objective")
	virtual bool FailObjective();

	UFUNCTION(BlueprintCallable, Category = "Objective")
	virtual void DeactivateTick();

	UFUNCTION(BlueprintCallable, Category = "Objective")
	bool IsObjectiveCompleted();

	UFUNCTION(BlueprintCallable, Category = "Objective")
	bool IsObjectiveFailed();

	UFUNCTION(BlueprintCallable, Category = "Objective")
	bool IsObjectiveActive();

	UFUNCTION(BlueprintNativeEvent, DisplayName = "Tick Objective", Category = "Objective")
	void TickObjective();
	virtual void TickObjective_Implementation();

	/**Add amount and check if the required amount has fulfilled, then complete the objective*/
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void AddAmount(int AmountToAdd = 1);

	virtual void Initialize(bool bLaunchEventOnLoad = false) override;
	virtual void Deinitialize() override;
	virtual void Reset() override;


	virtual void BeginNode() override;
	virtual void EvaluateNextNode() override;

#if WITH_EDITOR
	virtual FText GetNodeTitle() const override;
	virtual void SetNodeTitle(const FText& NewTitle);
	virtual FText GetNodeDescription() const override;
#endif
};
