// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "QuestSystemGraphNode.h"
#include "QuestSystemGraphNode_Objective.generated.h"

class UQuestTask;

UCLASS(Blueprintable)
class QUEST_SYSTEM_RUNTIME_API UQuestSystemGraphNode_Objective : public UQuestSystemGraphNode
{
	GENERATED_BODY()

public:
	/**List of task for your objective*/
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "Objective", meta = (EditFixedOrder))
		TArray<TObjectPtr<UQuestTask>> Tasks;

	UFUNCTION(BlueprintCallable, Category = "Objective")
		bool IsAllTaskCompleted();


#if WITH_EDITOR
	virtual FText GetNodeTitle() const override;
	virtual void SetNodeTitle(const FText& NewTitle);
	virtual FText GetNodeDescription() const override;
#endif
};
