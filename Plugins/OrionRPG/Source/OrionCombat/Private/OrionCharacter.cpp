// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionCharacter.h"

#include "AbilitySystem/OrionAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/OrionDefaultAttributeSet.h"


// Sets default values
AOrionCharacter::AOrionCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AbilitySystemComponent = CreateDefaultSubobject<UOrionAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// These attribute sets will be detected by AbilitySystemComponent::InitializeComponent. Keeping a reference so that the sets don't get garbage collected before that.
	HealthSet = CreateDefaultSubobject<UOrionDefaultAttributeSet>(TEXT("HealthSet"));
}

// Called when the game starts or when spawned
void AOrionCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AOrionCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AOrionCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

// Called to bind functionality to input
void AOrionCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

