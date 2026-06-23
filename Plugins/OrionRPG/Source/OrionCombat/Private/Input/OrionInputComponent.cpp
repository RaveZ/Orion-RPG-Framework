// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Input/OrionInputComponent.h"

#include "EnhancedInputSubsystems.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OrionInputComponent)

class UOrionInputConfig;

UOrionInputComponent::UOrionInputComponent(const FObjectInitializer& ObjectInitializer)
{
}

void UOrionInputComponent::AddInputMappings(const UOrionInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to add something from your input config if required
}

void UOrionInputComponent::RemoveInputMappings(const UOrionInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to remove input mappings that you may have added above
}

void UOrionInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}
