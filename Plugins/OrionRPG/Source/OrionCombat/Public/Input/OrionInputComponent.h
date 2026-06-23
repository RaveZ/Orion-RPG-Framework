// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "EnhancedInputComponent.h"
#include "OrionInputConfig.h"

#include "OrionInputComponent.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
class UObject;


/**
 * UOrionInputComponent
 *
 *	Component used to manage input mappings and bindings using an input config data asset.
 */
UCLASS(Config = Input)
class UOrionInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:

	UOrionInputComponent(const FObjectInitializer& ObjectInitializer);

	void AddInputMappings(const UOrionInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;
	void RemoveInputMappings(const UOrionInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

	template<class UserClass, typename FuncType>
	void BindNativeAction(const UOrionInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound);

	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityActions(const UOrionInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles);

	void RemoveBinds(TArray<uint32>& BindHandles);
};


template<class UserClass, typename FuncType>
void UOrionInputComponent::BindNativeAction(const UOrionInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
{
	check(InputConfig);
	if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
	{
		BindAction(IA, TriggerEvent, Object, Func);
	}
}

template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UOrionInputComponent::BindAbilityActions(const UOrionInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles)
{
	check(InputConfig);

	for (const FOrionInputAction& Action : InputConfig->AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			if (PressedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, PressedFunc, Action.InputTag).GetHandle());
			}

			if (ReleasedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag).GetHandle());
			}
		}
	}
}
