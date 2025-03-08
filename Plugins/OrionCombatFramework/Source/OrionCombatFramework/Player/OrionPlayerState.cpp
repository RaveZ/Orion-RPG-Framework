// Fill out your copyright notice in the Description page of Project Settings.


#include "OrionPlayerState.h"

#include "Net/UnrealNetwork.h"
#include "OrionCombatFramework/AbilitySystem/OrionAbilitySystemComponent.h"
#include "OrionCombatFramework/AbilitySystem/OrionAttributeSet.h"


AOrionPlayerState::AOrionPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UOrionAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	AttributeSet = CreateDefaultSubobject<UOrionAttributeSet>("AttributeSet");
	SetNetUpdateFrequency(100.f);
}

void AOrionPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AOrionPlayerState, Level);
}

UAbilitySystemComponent* AOrionPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AOrionPlayerState::OnRep_Level(int32 OldLevel)
{
}
