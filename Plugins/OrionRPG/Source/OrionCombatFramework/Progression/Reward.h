// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ScalableFloat.h"
#include "Reward.generated.h"


USTRUCT(BlueprintType)
struct FRewardInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FScalableFloat Amount = FScalableFloat();

public:
	FRewardInfo()
		: Amount(0.f)
	{
	}

	FRewardInfo(FGameplayTag InTag, FScalableFloat InAmount)
		: Tag(InTag)
		, Amount(InAmount)
	{
	}


	FORCEINLINE bool operator==(const FRewardInfo& Rhs) const
	{
		return Tag == Rhs.Tag;
	}

	FORCEINLINE bool operator==(const FGameplayTag& Rhs) const
	{
		return Tag == Rhs;
	}
};