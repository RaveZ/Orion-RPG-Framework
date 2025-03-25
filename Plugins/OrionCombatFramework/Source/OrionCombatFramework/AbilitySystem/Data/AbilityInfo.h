// Copyright 2025 Ivan Chandra. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AbilityInfo.generated.h"

USTRUCT(BlueprintType)
struct FOrionAbilityInfo
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag AbilityTag = FGameplayTag();

    UPROPERTY(BlueprintReadOnly)
    FGameplayTag InputTag = FGameplayTag();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag CooldownTag = FGameplayTag();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<const UTexture2D> Icon = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<const UMaterialInterface> BackgroundMaterial = nullptr;
};

/**
 *
 */
UCLASS()
class ORIONCOMBATFRAMEWORK_API UAbilityInfo : public UDataAsset
{
    GENERATED_BODY()
public:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilityInformation")
    TArray<FOrionAbilityInfo> AbilityInformation;

    FOrionAbilityInfo FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound = false) const;
};