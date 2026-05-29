// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "OrionAbilitySystemComponent.h"

UOrionAbilitySystemComponent::UOrionAbilitySystemComponent()
{
	SetIsReplicatedByDefault(true);
}

void UOrionAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);
}