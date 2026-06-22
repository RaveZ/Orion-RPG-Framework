// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "OrionDamageExecutionCalculation.generated.h"

/**
 * Converts incoming SetByCaller Gameplay.Damage into -Health after target defense mitigation.
 *
 * Use this as a GameplayEffect execution calculation when you want the GE to
 * calculate damage centrally. Existing effects that directly modify Damage are
 * mitigated separately in UOrionAttributeSetBase::PostGameplayEffectExecute.
 */
UCLASS()
class ORIONCOMBAT_API UOrionDamageExecutionCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UOrionDamageExecutionCalculation();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
