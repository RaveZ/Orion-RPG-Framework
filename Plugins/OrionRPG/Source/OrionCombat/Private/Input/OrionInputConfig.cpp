// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Input/OrionInputConfig.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_InputTag, "InputTag");


const UInputAction* UOrionInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag) const
{
	for (const FOrionInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}
	UE_LOG(LogTemp, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	
	return nullptr;
}

const UInputAction* UOrionInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag) const
{
	for (const FOrionInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}
	UE_LOG(LogTemp, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	
	return nullptr;
}