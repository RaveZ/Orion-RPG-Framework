// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "AbilitySystem/OrionAbilitySystemComponent.h"

#include "AbilitySystem/Attributes/OrionDefaultAttributeSet.h"

UOrionAbilitySystemComponent::UOrionAbilitySystemComponent()
{
	SetIsReplicatedByDefault(true);
	SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void UOrionAbilitySystemComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UOrionAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
	GetOrCreateAttributeSubobject(UOrionDefaultAttributeSet::StaticClass());
}

void UOrionAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);
}
