// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionCharacter.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "OrionCombatFramework/AbilitySystem/OrionAbilitySystemComponent.h"
#include "OrionCombatFramework/Player/OrionPlayerController.h"
#include "OrionCombatFramework/Player/OrionPlayerState.h"
#include "OrionCombatFramework/UI/HUD/OrionHUD.h"
#include "OrionCombatFramework/AbilitySystem/OrionAttributeSet.h"
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
		AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(TAG_Event_Reward).AddUObject(this, &AOrionCharacter::RewardEventCallback);
		AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(TAG_Event_AttributeUpgrade).AddUObject(this, &AOrionCharacter::AttributeUpgradeEventCallback);
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
			const int32 SkillPointsReward = Execute_GetSkillPointsReward(this, CurrentLevel);

			Execute_AddToPlayerLevel(this, NumLevelUps);
			Execute_AddToAttributePoints(this, AttributePointsReward);
			Execute_AddToSkillPoints(this, SkillPointsReward);

			Execute_LevelUp(this);
		}

		Execute_AddToXP(this, LocalIncomingXP);
	}
	if (Payload->EventTag.MatchesTag(TAG_Item))
	{
		//TODO : Add Item to Inventory
	}
}

void AOrionCharacter::AttributeUpgradeEventCallback(const FGameplayEventData* Payload)
{
	UOrionAttributeSet* AS = CastChecked<UOrionAttributeSet>(AttributeSet);
	check(AS);

	for (auto& Pair : AS->TagsToAttributes)
	{
		const FGameplayTag AttributeTag = Pair.Key;
		FGameplayAttribute Attribute = Pair.Value();

		
		if (Payload->EventTag.MatchesTagExact(AttributeTag))
		{
			const float Magnitude = Payload->EventMagnitude;
			Attribute.GetGameplayAttributeData(AS)->SetBaseValue(Attribute.GetGameplayAttributeData(AS)->GetBaseValue() + Magnitude);
			break;
		}
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

int32 AOrionCharacter::GetSkillPointsReward_Implementation(int32 Level) const
{
	const AOrionPlayerState* OrionPlayerState = GetPlayerState<AOrionPlayerState>();
	if (OrionPlayerState == nullptr)
	{
		UE_LOG(LogOrion, Warning, TEXT("OrionCharacter: OrionPlayerState is null!"));
		return int32();
	}
	return OrionPlayerState->LevelUpInfo->LevelUpInformation[Level].SkillPointAward;
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
	AOrionPlayerState* OrionPlayerState = GetPlayerState<AOrionPlayerState>();
	if (OrionPlayerState == nullptr)
	{
		UE_LOG(LogOrion, Warning, TEXT("OrionCharacter: OrionPlayerState is null!"));
	}
	OrionPlayerState->AddToAttributePoints(InAttributePoints);
}

void AOrionCharacter::AddToSkillPoints_Implementation(int32 InSkillPoints)
{
	AOrionPlayerState* OrionPlayerState = GetPlayerState<AOrionPlayerState>();
	if (OrionPlayerState == nullptr)
	{
		UE_LOG(LogOrion, Warning, TEXT("OrionCharacter: OrionPlayerState is null!"));
	}
	OrionPlayerState->AddToSkillPoints(InSkillPoints);
}

int32 AOrionCharacter::GetAttributePoints_Implementation() const
{
	AOrionPlayerState* OrionPlayerState = GetPlayerState<AOrionPlayerState>();
	if (OrionPlayerState == nullptr)
	{
		UE_LOG(LogOrion, Warning, TEXT("OrionCharacter: OrionPlayerState is null!"));
	}
	return OrionPlayerState->GetAttributePoints();
}

int32 AOrionCharacter::GetSkillPoints_Implementation() const
{
	AOrionPlayerState* OrionPlayerState = GetPlayerState<AOrionPlayerState>();
	if (OrionPlayerState == nullptr)
	{
		UE_LOG(LogOrion, Warning, TEXT("OrionCharacter: OrionPlayerState is null!"));
	}
	return OrionPlayerState->GetSkillPoints();
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
