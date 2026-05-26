// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "OrionInputConfig.generated.h"


USTRUCT(BlueprintType)
struct FOrionInputAction
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly)
    const class UInputAction* InputAction = nullptr;

    UPROPERTY(EditDefaultsOnly)
    FGameplayTag InputTag = FGameplayTag();
};

UCLASS()
class ORIONCOMBATFRAMEWORK_API UOrionInputConfig : public UDataAsset
{
	GENERATED_BODY()
public:
    const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = false) const;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FOrionInputAction> AbilityInputActions; 
};
