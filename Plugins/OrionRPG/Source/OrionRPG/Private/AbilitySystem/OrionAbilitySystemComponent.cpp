// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "AbilitySystem/OrionAbilitySystemcomponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystem/OrionGlobalAbilitySystem.h"

UOrionAbilitySystemComponent::UOrionAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();

}

void UOrionAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UOrionGlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<UOrionGlobalAbilitySystem>(GetWorld()))
	{
		GlobalAbilitySystem->UnregisterASC(this);
	}

	Super::EndPlay(EndPlayReason);
}

void UOrionAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	check(ActorInfo);
	check(InOwnerActor);

	const bool bHasNewPawnAvatar = Cast<APawn>(InAvatarActor) && (InAvatarActor != ActorInfo->AvatarActor);

	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	//if (bHasNewPawnAvatar)
	//{
	//	// Notify all abilities that a new pawn avatar has been set
	//	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	//	{
	//		PRAGMA_DISABLE_DEPRECATION_WARNINGS
	//			ensureMsgf(AbilitySpec.Ability && AbilitySpec.Ability->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced, TEXT("InitAbilityActorInfo: All Abilities should be Instanced (NonInstanced is being deprecated due to usability issues)."));
	//		PRAGMA_ENABLE_DEPRECATION_WARNINGS

	//			TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
	//		for (UGameplayAbility* AbilityInstance : Instances)
	//		{
	//			UOrionGameplayAbility* OrionAbilityInstance = Cast<UOrionGameplayAbility>(AbilityInstance);
	//			if (OrionAbilityInstance)
	//			{
	//				// Ability instances may be missing for replays
	//				OrionAbilityInstance->OnPawnAvatarSet();
	//			}
	//		}
	//	}

	//	// Register with the global system once we actually have a pawn avatar. We wait until this time since some globally-applied effects may require an avatar.
	//	if (UOrionGlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<UOrionGlobalAbilitySystem>(GetWorld()))
	//	{
	//		GlobalAbilitySystem->RegisterASC(this);
	//	}

	//	if (UOrionAnimInstance* OrionAnimInst = Cast<UOrionAnimInstance>(ActorInfo->GetAnimInstance()))
	//	{
	//		OrionAnimInst->InitializeWithAbilitySystem(this);
	//	}

	//	TryActivateAbilitiesOnSpawn();
	//}
}
