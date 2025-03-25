// Copyright 2025 Ivan Chandra. All Rights Reserved.


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
	DOREPLIFETIME(AOrionPlayerState, XP);
}

UAbilitySystemComponent* AOrionPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AOrionPlayerState::AddToXP(int32 InXP)
{
	XP += InXP;
	OnXPChangedDelegate.Broadcast(XP);
}

void AOrionPlayerState::AddToLevel(int32 InLevel)
{
	Level += InLevel;
	OnLevelChangedDelegate.Broadcast(Level);	
}

void AOrionPlayerState::SetXP(int32 InXP)
{
	XP = InXP;
	OnXPChangedDelegate.Broadcast(XP);
}

void AOrionPlayerState::SetLevel(int32 InLevel)
{
	Level = InLevel;
	OnLevelChangedDelegate.Broadcast(Level);
}

void AOrionPlayerState::OnRep_Level(int32 OldLevel)
{
	OnLevelChangedDelegate.Broadcast(Level);
}

void AOrionPlayerState::OnRep_XP(int32 OldXP)
{
	OnXPChangedDelegate.Broadcast(XP);
}
