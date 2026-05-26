// Copyright 2025 Ivan Chandra. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BehaviorTree.h"
#include "OrionAIController.generated.h"

/**
 * 
 */
UCLASS()
class ORIONCOMBATFRAMEWORK_API AOrionAIController : public AAIController
{
	GENERATED_BODY()

public:
	AOrionAIController();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> Behaviour = nullptr;

protected:
	virtual void OnPossess(APawn* InPawn) override;

	virtual FVector GetFocalPointOnActor(const AActor *Actor) const override;
};
