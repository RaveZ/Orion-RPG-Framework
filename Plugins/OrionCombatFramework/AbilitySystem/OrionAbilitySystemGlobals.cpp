// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionAbilitySystemGlobals.h"
#include "OrionAbilityTypes.h"

FGameplayEffectContext* UOrionAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	//to use custom gameplayeffectcontext, return your custom effect context here
	return new FOrionGameplayEffectContext();	
}
