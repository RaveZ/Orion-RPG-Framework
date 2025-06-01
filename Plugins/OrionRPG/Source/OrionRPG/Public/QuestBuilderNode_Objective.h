// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderNode_Objective.generated.h"

class UQuestTask;

UCLASS(Blueprintable)
class ORIONRPG_API UQuestBuilderNode_Objective : public UQuestBuilderNode
{
	GENERATED_BODY()

public:
	/**List of task for your objective*/
	UPROPERTY(Instanced, BlueprintReadOnly, Category = "Objective", meta = (FullyExpand = true))
		TArray<TObjectPtr<UQuestTask>> Tasks;

	virtual void Initialize(bool bLaunchEventOnLoad = false) override;
	virtual void Deinitialize() override;
	virtual void Reset() override;

	void BeginAllTask();

	UFUNCTION(BlueprintCallable, Category = "Objective")
		bool IsAllTaskCompleted();

	virtual void BeginNode() override;
	virtual void EvaluateNextNode() override;

#if WITH_EDITOR
	virtual FText GetNodeTitle() const override;
	virtual void SetNodeTitle(const FText& NewTitle);
	virtual FText GetNodeDescription() const override;
#endif
};
