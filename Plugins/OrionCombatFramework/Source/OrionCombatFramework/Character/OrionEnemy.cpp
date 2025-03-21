// Fill out your copyright notice in the Description page of Project Settings.


#include "OrionEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "OrionCombatFramework/OrionCombatFramework.h"
#include "OrionCombatFramework/AbilitySystem/OrionAbilitySystemComponent.h"
#include "OrionCombatFramework/AbilitySystem/OrionAttributeSet.h"
#include "../UI/Widget/OrionUserWidget.h"
#include "../OrionGameplayTags.h"

AOrionEnemy::AOrionEnemy()
{
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	AbilitySystemComponent = CreateDefaultSubobject<UOrionAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UOrionAttributeSet>("AttributeSet");

	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());
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

void AOrionEnemy::Die()
{
    SetLifeSpan(LifeSpan);
    Super::Die();
}

void AOrionEnemy::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
    bHitReacting = NewCount > 0;
    GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;
}


void AOrionEnemy::BeginPlay()
{
	Super::BeginPlay();
    GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	InitAbilityActorInfo();
    AddCharacterAbilities();

    if (UOrionUserWidget* OrionUserWidget = Cast<UOrionUserWidget>(HealthBar->GetUserWidgetObject()))
    {
        OrionUserWidget->SetWidgetController(this);
    }

    if (const UOrionAttributeSet* OrionAS = Cast<UOrionAttributeSet>(AttributeSet))
    {
        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(OrionAS->GetHealthAttribute()).AddLambda(
            [this, OrionAS](const FOnAttributeChangeData& Data)
            {
                OnHealthChanged.Broadcast(Data.NewValue);
            }
        );
        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(OrionAS->GetMaxHealthAttribute()).AddLambda(
            [this](const FOnAttributeChangeData& Data)
            {
                OnMaxHealthChanged.Broadcast(Data.NewValue);
            }
        );
        AbilitySystemComponent->RegisterGameplayTagEvent(TAG_Effects_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(
            this,
            &AOrionEnemy::HitReactTagChanged
        );

        OnHealthChanged.Broadcast(OrionAS->GetHealth());
        OnMaxHealthChanged.Broadcast(OrionAS->GetMaxHealth());
    }
}

void AOrionEnemy::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<UOrionAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();

	InitializeDefaultAttributes();
}
