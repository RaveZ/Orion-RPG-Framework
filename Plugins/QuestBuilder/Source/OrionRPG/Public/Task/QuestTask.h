// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"
#include "UObject/NoExportTypes.h"
#include "QuestTask.generated.h"


class UQuestBuilderGraph;
class UQuest;
class UQuestComponent;
class UQuestBuilderNode_Objective;




UENUM(BlueprintType)
enum class ETaskState : uint8
{
	E_Active		UMETA(DisplayName = "ACTIVE"),
	E_Complete		UMETA(DisplayName = "COMPLETE"),
};
UCLASS(Abstract, Blueprintable, EditInlineNew, AutoExpandCategories = ("Default"))
class ORIONRPG_API UQuestTask : public UObject
{
	GENERATED_BODY()

public:
	UQuestTask();

	/** node name */
	UPROPERTY(Category = Description, EditAnywhere)
	FString NodeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (MultiLine = true))
		FText Description;

	/**mark the task as optional and skip this task if the other non - optional task has been completed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task")
		bool bOptional;

	/**Hide the task from widget*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task")
		bool bHidden;
	
	/**This task use amount required to complete task?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "UseAmount"))
		bool bUseAmount;

	/**Amount required for your task*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (EditCondition = "bUseAmount == true", EditConditionHides))
		int RequiredAmount;

	/**Is Task Tickable?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (DisplayName = "TaskTickable"))
		bool bIsTaskTickable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task", meta = (EditCondition = "bIsTaskTickable == true", EditConditionHides))
		float TickInterval;

	UPROPERTY(BlueprintReadOnly, Category = "Task")
		TObjectPtr<UQuestBuilderGraph> QuestGraph;

	UPROPERTY(BlueprintReadOnly, Category = "Task")
		TObjectPtr<UQuest> Quest;


	UPROPERTY(BlueprintReadOnly, Category = "Task")
		TObjectPtr<UQuestBuilderNode_Objective> ObjectiveNode;

	UPROPERTY(BlueprintReadWrite, Category = "Task")
		ETaskState TaskState = ETaskState::E_Active;

	UPROPERTY(BlueprintReadWrite, Category = "Task")
		int CurrentAmount = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Task")
		float DistanceToTarget;

	FTimerHandle TimerHandle_TickTask;

public:

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Begin Task", Category = "Task")
		void K2_BeginTask(AController* OwnerController, APawn* ControlledPawn);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Task Completed", Category = "Task")
		void K2_TaskCompleted(AController* OwnerController, APawn* ControlledPawn);

	UFUNCTION(BlueprintCallable, Category = "Task")
		virtual void BeginTask();

	UFUNCTION(BlueprintCallable, Category = "Task")
		virtual bool CompleteTask();

	UFUNCTION(BlueprintCallable, Category = "Task")
		virtual void Deactivate();

	UFUNCTION(BlueprintCallable, Category = "Objective")
		bool IsTaskCompleted();

	UFUNCTION(BlueprintNativeEvent, DisplayName = "Tick Task", Category = "Task")
		void TickTask();
	virtual void TickTask_Implementation();

	UFUNCTION(BlueprintCallable, Category = "Task")
		AController* GetOwningController() const;

	UFUNCTION(BlueprintCallable, Category = "Task")
		APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintCallable, Category = "Task")
		UQuestComponent* GetQuestComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Task")
		UQuestBuilderGraph* GetOwningQuestGraph() const;

	UFUNCTION(BlueprintCallable, Category = "Task")
		UQuest* GetOwningQuest() const;

	/**Add amount and check if the required amount has fulfilled, then complete the task*/
	UFUNCTION(BlueprintCallable, Category = "Task")
		void AddAmount(int AmountToAdd = 1);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Task")
		FText GetDescription() const;

	virtual FText GetDescription_Implementation() const;


	/** @return name of node */
	FString GetNodeName();
	FString GetShortTypeName(UObject* Ob);

	// Allows the Object to get a valid UWorld from it's outer.
	virtual UWorld* GetWorld() const override;
	
};
