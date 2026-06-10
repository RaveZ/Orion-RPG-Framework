// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "AbilitySystem/OrionAbilitySystemComponent.h"

#include "AbilitySystem/Attributes/OrionAttributeSetBase.h"

UOrionAbilitySystemComponent::UOrionAbilitySystemComponent()
{
	SetIsReplicatedByDefault(true);
	DefaultAttributeSetClass = UOrionAttributeSetBase::StaticClass();
	SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void UOrionAbilitySystemComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UOrionAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UOrionAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UOrionAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);
	
	if (DefaultAttributeSetClass)
	{
		const UAttributeSet* AttributeSet = GetOrCreateAttributeSubobject(DefaultAttributeSetClass);
		DefaultAttributeSet = const_cast<UOrionAttributeSetBase*>(Cast<UOrionAttributeSetBase>(AttributeSet));
		
	}
	OnInitAbilityActorInfo.Broadcast();
}
