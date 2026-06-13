// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "OrionTargetComponent.generated.h"

/**
 * 
 */
UCLASS(meta = (BlueprintSpawnableComponent), Blueprintable, HideDropdown, ClassGroup = "OrionRPG", DisplayName = "Orion Target")
class ORIONCOMBAT_API UOrionTargetComponent : public USceneComponent
{
	GENERATED_BODY()
	
public:
	//Owning Pawn
	UPROPERTY(BlueprintReadOnly, Category = "TargetingSystem")
	TObjectPtr<APawn> OwningPawn;
	
public:
	
	UOrionTargetComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintPure, Category = "TargetingSystem")
	virtual APawn* GetOwningPawn() const;
};
