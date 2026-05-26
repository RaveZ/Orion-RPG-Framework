// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionCharacter.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../AbilitySystem/OrionAbilitySystemComponent.h"
#include "../Player/OrionPlayerController.h"
#include "../Player/OrionPlayerState.h"
#include "../UI/HUD/OrionHUD.h"
#include "../AbilitySystem/OrionAttributeSet.h"
#include "../Progression/LevelUpInfo.h"
#include "../OrionLogChannels.h"
#include "../OrionGameplayTags.h"
#include "../AI/OrionAIController.h"

#include "NiagaraComponent.h"
#include "Camera/CameraComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Components/SkeletalMeshComponent.h"

AOrionCharacter::AOrionCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	HeldObjectRoot = CreateDefaultSubobject<USceneComponent>(TEXT("HeldObjectRoot"));
	HeldObjectRoot->SetupAttachment(GetMesh());

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(HeldObjectRoot);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(HeldObjectRoot);

	AIControllerClass = AOrionAIController::StaticClass();

	LevelUpNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("LevelUpNiagaraComponent");
	LevelUpNiagaraComponent->SetupAttachment(GetRootComponent());
	LevelUpNiagaraComponent->bAutoActivate = false;
}

void AOrionCharacter::BeginPlay()
{
	Super::BeginPlay();
	UpdateHeldObject();
	
}

void AOrionCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateHeldObjectAnimations();
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
	//if (Payload->EventTag.MatchesTag(TAG_Item))
	//{
	//	//TODO : Add Item to Inventory
	//}
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
 		const FVector MeshLocation = GetMesh()->GetComponentLocation();
 		const FVector NiagaraSystemLocation = LevelUpNiagaraComponent->GetComponentLocation();
 		const FRotator ToCameraRotation = (MeshLocation - NiagaraSystemLocation).Rotation();
 		LevelUpNiagaraComponent->SetWorldRotation(ToCameraRotation);
 		LevelUpNiagaraComponent->Activate(true);
 	}
}


void AOrionCharacter::ClearHeldObject()
{
	StaticMesh->SetStaticMesh(nullptr);
	SkeletalMesh->SetSkeletalMesh(nullptr);
	SkeletalMesh->SetAnimInstanceClass(nullptr);
}

void AOrionCharacter::AttachToHand(UStaticMesh* NewStaticMesh, USkeletalMesh* NewSkeletalMesh, UClass* NewAnimClass,
	bool bLeftHand, FVector Offset)
{
	ClearHeldObject();

	if (IsValid(NewStaticMesh))
	{
		StaticMesh->SetStaticMesh(NewStaticMesh);
	}
	else if (IsValid(NewSkeletalMesh))
	{
		SkeletalMesh->SetSkeletalMesh(NewSkeletalMesh);
		if (IsValid(NewAnimClass))
		{
			SkeletalMesh->SetAnimInstanceClass(NewAnimClass);
		}
	}

	FName AttachBone;
	if (bLeftHand)
	{
		AttachBone = TEXT("VB LHS_ik_hand_gun");
	}
	else
	{
		AttachBone = TEXT("VB RHS_ik_hand_gun");
	}

	HeldObjectRoot->AttachToComponent(GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachBone);
	HeldObjectRoot->SetRelativeLocation(Offset);
}

void AOrionCharacter::RagdollStart()
{
	ClearHeldObject();
	Super::RagdollStart();
}

void AOrionCharacter::RagdollEnd()
{
	Super::RagdollEnd();
	UpdateHeldObject();
}

ECollisionChannel AOrionCharacter::GetThirdPersonTraceParams(FVector& TraceOrigin, float& TraceRadius)
{
	const FName CameraSocketName = bRightShoulder ? TEXT("TP_CameraTrace_R") : TEXT("TP_CameraTrace_L");
	TraceOrigin = GetMesh()->GetSocketLocation(CameraSocketName);
	TraceRadius = 15.0f;
	return ECC_Camera;
}

FTransform AOrionCharacter::GetThirdPersonPivotTarget()
{
	return FTransform(GetActorRotation(),
		(GetMesh()->GetSocketLocation(TEXT("Head")) + GetMesh()->GetSocketLocation(TEXT("root"))) / 2.0f,
		FVector::OneVector);
}

FVector AOrionCharacter::GetFirstPersonCameraTarget()
{
	return GetMesh()->GetSocketLocation(TEXT("FP_Camera"));
}

void AOrionCharacter::OnOverlayStateChanged(EOrionOverlayState PreviousState)
{
	Super::OnOverlayStateChanged(PreviousState);
	UpdateHeldObject();
}