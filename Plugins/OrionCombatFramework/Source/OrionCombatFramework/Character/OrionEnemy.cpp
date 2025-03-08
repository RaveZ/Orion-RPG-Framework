// Fill out your copyright notice in the Description page of Project Settings.


#include "OrionEnemy.h"

#include "OrionCombatFramework/OrionCombatFramework.h"
#include "OrionCombatFramework/AbilitySystem/OrionAbilitySystemComponent.h"
#include "OrionCombatFramework/AbilitySystem/OrionAttributeSet.h"

AOrionEnemy::AOrionEnemy()
{
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	AbilitySystemComponent = CreateDefaultSubobject<UOrionAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UOrionAttributeSet>("AttributeSet");
}

void AOrionEnemy::HighlightActor()
{
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	Weapon->SetRenderCustomDepth(true);
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
}

void AOrionEnemy::UnHighlightActor()
{
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);
}

int32 AOrionEnemy::GetPlayerLevel()
{
	return Level;
}


void AOrionEnemy::BeginPlay()
{
	Super::BeginPlay();
	InitAbilityActorInfo();
}

void AOrionEnemy::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<UOrionAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
}
