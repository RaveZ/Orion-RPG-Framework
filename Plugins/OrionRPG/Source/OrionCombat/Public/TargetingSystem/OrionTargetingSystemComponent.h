// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OrionTargetingSystemComponent.generated.h"

class UOrionTargetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrionTargetingSignature, UOrionTargetComponent*, Target);

UCLASS(meta = (BlueprintSpawnableComponent), Blueprintable, HideDropdown, ClassGroup = "OrionRPG", DisplayName = "Orion Targeting System")
class ORIONCOMBAT_API UOrionTargetingSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//Owning Pawn
	UPROPERTY(BlueprintReadOnly, Category = "TargetingSystem")
	TObjectPtr<APawn> OwningPawn;
	
	UPROPERTY(BlueprintReadOnly, Category = "TargetingSystem")
	TObjectPtr<UOrionTargetComponent> CurrentTarget;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TargetingSystem")
	bool bTargetingEnabled;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	TEnumAsByte<ECollisionChannel> TraceChannel;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetingSystem")
	float MaxDistance;
	
	UPROPERTY(BlueprintAssignable, Category = "TargetingSystem")
	FOrionTargetingSignature OnLostTarget;
	
	UPROPERTY(BlueprintAssignable, Category = "TargetingSystem")
	FOrionTargetingSignature OnTargetChanged;
	
public:
	UOrionTargetingSystemComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	
	UFUNCTION(BlueprintPure, Category = "TargetingSystem")
	virtual APawn* GetOwningPawn() const;
	
	UFUNCTION()
	float GetDistanceToTarget();
	
	UFUNCTION(BlueprintCallable, Category = "TargetingSystem")
	void SetEnableTargeting(bool bEnable);
	
	UFUNCTION()
	void SearchTarget();
	
	
};
