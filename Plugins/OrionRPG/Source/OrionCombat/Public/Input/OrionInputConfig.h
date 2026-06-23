// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "Engine/DataAsset.h"
#include "OrionInputConfig.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InputTag);

class UInputAction;
/**
 * FOrionInputAction
 *
 *	Struct used to map an input action to a gameplay input tag.
 */
USTRUCT(BlueprintType)
struct FOrionInputAction
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	const UInputAction* InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

UCLASS()
class ORIONCOMBAT_API UOrionInputConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag) const;
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag) const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FOrionInputAction> NativeInputActions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FOrionInputAction> AbilityInputActions;
};
