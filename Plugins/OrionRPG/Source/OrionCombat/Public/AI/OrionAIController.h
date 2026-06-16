// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "OrionAIController.generated.h"

UCLASS()
class ORIONCOMBAT_API AOrionAIController : public AAIController
{
	GENERATED_BODY()

public:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStateTreeAIComponent> StateTreeAIComponent;
	
	
	UPROPERTY(EditAnywhere, Category = "AI", BlueprintReadWrite)
	float SmoothFocusInterpSpeed = 65.f;
	
	/**Target actor*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	TObjectPtr<AActor> TargetActor;
	
public:
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Target Set", Category = "AI")
	void K2_OnTargetSet(AActor* InTarget);

	UFUNCTION(BlueprintCallable, Category = "AI")
	virtual void SetTarget(AActor* InTarget, const bool bSetFocus = true);

	UFUNCTION(BlueprintCallable, Category = "AI", meta = (AllowPrivateAccess = true))
	void PerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION(BlueprintCallable, Category = "AI")
	void PauseLogic();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void ResumeLogic();
	
protected:
	AOrionAIController();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn = true) override;

protected:
	FRotator SmoothTargetRotation;

};
