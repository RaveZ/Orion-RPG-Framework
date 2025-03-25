// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionCharacter.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "OrionCombatFramework/AbilitySystem/OrionAbilitySystemComponent.h"
#include "OrionCombatFramework/Player/OrionPlayerController.h"
#include "OrionCombatFramework/Player/OrionPlayerState.h"
#include "OrionCombatFramework/UI/HUD/OrionHUD.h"


AOrionCharacter::AOrionCharacter()
{
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	
}

void AOrionCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AOrionCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	//Init Ability Actor info to the Server
	InitAbilityActorInfo();
	AddCharacterAbilities();
}

void AOrionCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	//Init Ability Actor info to the Client
	InitAbilityActorInfo();
}

void AOrionCharacter::AddToXP_Implementation(int32 InXP)
{
	AOrionPlayerState* OrionPlayerState = GetPlayerState<AOrionPlayerState>();
	check(OrionPlayerState);
	OrionPlayerState->AddToXP(InXP);
}

void AOrionCharacter::LevelUp_Implementation()
{
}

int32 AOrionCharacter::GetPlayerLevel_Implementation()
{
	const AOrionPlayerState* OrionPlayerState = GetPlayerState<AOrionPlayerState>();
	check(OrionPlayerState);
	return OrionPlayerState->GetPlayerLevel();
}

void AOrionCharacter::InitAbilityActorInfo()
{
	AOrionPlayerState* OrionPlayerState = GetPlayerState<AOrionPlayerState>();
	check(OrionPlayerState);
	OrionPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(OrionPlayerState, this);
	Cast<UOrionAbilitySystemComponent>(OrionPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	AbilitySystemComponent = OrionPlayerState->GetAbilitySystemComponent();
	AttributeSet = OrionPlayerState->GetAttributeSet();

	if (AOrionPlayerController* OrionPlayerController = Cast<AOrionPlayerController>(GetController()))
	{
		if (AOrionHUD* OrionHUD = Cast<AOrionHUD>(OrionPlayerController->GetHUD()))
		{
			OrionHUD->InitOverlay(OrionPlayerController, OrionPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}
	InitializeDefaultAttributes();
}
