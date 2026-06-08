// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "AbilitySystem/Attributes//OrionAttributeSet.h"

#include "AbilitySystem/OrionAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

UOrionAttributeSet::UOrionAttributeSet()
{
}

UWorld* UOrionAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

UOrionAbilitySystemComponent* UOrionAttributeSet::GetOrionAbilitySystemComponent() const
{
	return Cast<UOrionAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}


