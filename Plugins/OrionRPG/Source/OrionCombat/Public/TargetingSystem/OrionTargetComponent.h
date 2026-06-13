// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "OrionTargetComponent.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent), Blueprintable, HideDropdown, ClassGroup = "OrionRPG", DisplayName = "Orion Target")
class ORIONCOMBAT_API UOrionTargetComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UOrionTargetComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	bool bIsTargetable = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	int32 TeamId = 1;  // 0 = neutral, 1 = enemy, 2 = ally, etc.

	UPROPERTY(BlueprintReadOnly, Category = "Targeting")
	TObjectPtr<APawn> OwningPawn;

public:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure, Category = "Targeting")
	virtual APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintPure, Category = "Targeting")
	bool IsTargetable() const { return bIsTargetable && OwningPawn /*&& OwningPawn->IsAlive()*/; } // assumes IsAlive exists
};