// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionCharacter.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "OrionCombatFramework/AbilitySystem/OrionAbilitySystemComponent.h"
#include "OrionCombatFramework/Player/OrionPlayerController.h"
#include "OrionCombatFramework/Player/OrionPlayerState.h"
#include "OrionCombatFramework/UI/HUD/OrionHUD.h"
#include "NiagaraComponent.h"
#include "Camera/CameraComponent.h"
#include "../OrionGameplayTags.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameFramework/SpringArmComponent.h"
#include "../Progression/LevelUpInfo.h"
#include "../OrionLogChannels.h"

AOrionCharacter::AOrionCharacter()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->bDoCollisionTest = false;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;

	LevelUpNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("LevelUpNiagaraComponent");
	LevelUpNiagaraComponent->SetupAttachment(GetRootComponent());
	LevelUpNiagaraComponent->bAutoActivate = false;

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

	// Listen for Wait Gameplay Event to Actor
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(TAG_Reward).AddUObject(this, &AOrionCharacter::RewardEventCallback);
	}
}

void AOrionCharacter::RewardEventCallback(const FGameplayEventData* Payload)
{
	if (Payload->EventTag.MatchesTagExact(TAG_Attributes_XP))
	{
		const float LocalIncomingXP = Payload->EventMagnitude;
		UE_LOG(LogOrion, Log, TEXT("Incoming XP: %f"), LocalIncomingXP);
		
		const int32 CurrentLevel = Execute_GetPlayerLevel(this);
		const int32 CurrentXP = Execute_GetXP(this);

		const int32 NewLevel = Execute_FindLevelForXP(this, CurrentXP + LocalIncomingXP);
		const int32 NumLevelUps = NewLevel - CurrentLevel;
		if (NumLevelUps > 0)
		{
			const int32 AttributePointsReward = Execute_GetAttributePointsReward(this, CurrentLevel);
			const int32 SpellPointsReward = Execute_GetSpellPointsReward(this, CurrentLevel);

			Execute_AddToPlayerLevel(this, NumLevelUps);
			Execute_AddToAttributePoints(this, AttributePointsReward);
			Execute_AddToSpellPoints(this, SpellPointsReward);

			Execute_LevelUp(this);
		}

		Execute_AddToXP(this, LocalIncomingXP);
	}
	if (Payload->EventTag.MatchesTag(TAG_Item))
	{
		//TODO : Add Item to Inventory
	}
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
	MulticastLevelUpParticles();
}

int32 AOrionCharacter::GetXP_Implementation() const
{
	const AOrionPlayerState* OrionPlayerState = GetPlayerState<AOrionPlayerState>();
	if (OrionPlayerState == nullptr)
	{
		UE_LOG(LogOrion, Warning, TEXT("OrionCharacter: OrionPlayerState is null!"));
		return int32();
	}
	return OrionPlayerState->GetXP();

}

int32 AOrionCharacter::FindLevelForXP_Implementation(int32 InXP) const
{
	const AOrionPlayerState* OrionPlayerState = GetPlayerState<AOrionPlayerState>();
	if (OrionPlayerState == nullptr)
	{
		UE_LOG(LogOrion, Warning, TEXT("OrionCharacter: OrionPlayerState is null!"));
		return int32();
	}
	return OrionPlayerState->LevelUpInfo->FindLevelForXP(InXP);
}

int32 AOrionCharacter::GetAttributePointsReward_Implementation(int32 Level) const
{
	const AOrionPlayerState* OrionPlayerState = GetPlayerState<AOrionPlayerState>();
	if (OrionPlayerState == nullptr)
	{
		UE_LOG(LogOrion, Warning, TEXT("OrionCharacter: OrionPlayerState is null!"));
		return int32();
	}
	return OrionPlayerState->LevelUpInfo->LevelUpInformation[Level].AttributePointAward;
}

int32 AOrionCharacter::GetSpellPointsReward_Implementation(int32 Level) const
{
	const AOrionPlayerState* OrionPlayerState = GetPlayerState<AOrionPlayerState>();
	if (OrionPlayerState == nullptr)
	{
		UE_LOG(LogOrion, Warning, TEXT("OrionCharacter: OrionPlayerState is null!"));
		return int32();
	}
	return OrionPlayerState->LevelUpInfo->LevelUpInformation[Level].SpellPointAward;
}

void AOrionCharacter::AddToPlayerLevel_Implementation(int32 InPlayerLevel)
{
	AOrionPlayerState* OrionPlayerState = GetPlayerState<AOrionPlayerState>();
	if (OrionPlayerState == nullptr)
	{
		UE_LOG(LogOrion, Warning, TEXT("OrionCharacter: OrionPlayerState is null!"));
	}
	OrionPlayerState->AddToLevel(InPlayerLevel);
}

void AOrionCharacter::AddToAttributePoints_Implementation(int32 InAttributePoints)
{

}

void AOrionCharacter::AddToSpellPoints_Implementation(int32 InSpellPoints)
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

void AOrionCharacter::MulticastLevelUpParticles_Implementation() const
{
	if (IsValid(LevelUpNiagaraComponent))
 	{
 		const FVector CameraLocation = CameraComponent->GetComponentLocation();
 		const FVector NiagaraSystemLocation = LevelUpNiagaraComponent->GetComponentLocation();
 		const FRotator ToCameraRotation = (CameraLocation - NiagaraSystemLocation).Rotation();
 		LevelUpNiagaraComponent->SetWorldRotation(ToCameraRotation);
 		LevelUpNiagaraComponent->Activate(true);
 	}
}
