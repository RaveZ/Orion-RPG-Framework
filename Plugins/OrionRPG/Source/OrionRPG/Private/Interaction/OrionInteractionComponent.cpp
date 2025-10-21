// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Interaction/OrionInteractionComponent.h"
#include "Interaction/OrionInteractionTargetComponent.h"
#include "CollisionQueryParams.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"



UOrionInteractionComponent::UOrionInteractionComponent()
{
	// Initialize variables
	bInteracting = false;
	InteractionTraceChannel = ECC_WorldDynamic;
	InteractionType = EOrionInteractionType::E_InteractionPawn;
	InteractionRadius = 150.f;
	InteractionTraceDistance = 750.0f;
	InteractionTraceIntervalSecond = 0.3f;
	InteractionOffset = FVector(75.f, 0.f, 0.f);
	bIsInteractionEnabled = false;
	bDebugTrace = false;

	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UOrionInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickInterval(InteractionTraceIntervalSecond);

	// Cache the owning controller
	OwningController = GetOwningController();

	if (OwningController)
	{
        // Fix: Use a delegate to bind the function properly
        OwningController->OnPossessedPawnChanged.AddDynamic(this, &UOrionInteractionComponent::OnPossessedPawnChanged);
     
	}
	SetInteractionEnabled(true);

	
}
void UOrionInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if ((EndPlayReason == EEndPlayReason::Destroyed ||
		EndPlayReason == EEndPlayReason::RemovedFromWorld) && bInteracting)
	{
		CancelInteraction();
	}
}

void UOrionInteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsInteractionEnabled);
	DOREPLIFETIME(ThisClass, InteractionIndex);
	DOREPLIFETIME(ThisClass, bInteracting);
}

void UOrionInteractionComponent::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (NewPawn == nullptr)
	{
		SetInteractionEnabled(false);
	}
	else
	{
		SetInteractionEnabled(true);
	}
}




void UOrionInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateInteractionTrace();

}

void UOrionInteractionComponent::OnRep_bInteracting()
{
	if (OnInteractingStateChanged.IsBound())
	{
		OnInteractingStateChanged.Broadcast(bInteracting);
	}
}

APawn* UOrionInteractionComponent::GetOwningPawn() const
{
	if (OwningController)
	{
		return OwningController->GetPawn();
	}

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	APawn* OwningPawn = Cast<APawn>(GetOwner());

	if (OwningPawn)
	{
		return OwningPawn;
	}

	if (!OwningPawn && PC)
	{
		return PC->GetPawn();
	}

	return nullptr;
}

APlayerController* UOrionInteractionComponent::GetOwningController() const
{
	if (OwningController)
	{
		return OwningController;
	}

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	APawn* OwningPawn = Cast<APawn>(GetOwner());

	if (PC)
	{
		return PC;
	}

	if (!PC && OwningPawn)
	{
		return Cast<APlayerController>(OwningPawn->GetController());
	}

	return nullptr;
}

void UOrionInteractionComponent::SetInteractionEnabled(bool bEnabled)
{
	if (bIsInteractionEnabled == bEnabled)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Set interaction enabled: %d"), bEnabled);

	bIsInteractionEnabled = bEnabled;
	UWorld* World = GetWorld();
	if (World)
	{
		if (bIsInteractionEnabled)
		{
			SetComponentTickEnabled(
				ShouldTickInstance()
			);
		}
		else
		{
			SetComponentTickEnabled(false);
			CancelInteraction();
			CurrentInteractionTarget = nullptr;

		}
	}
}

bool UOrionInteractionComponent::ValidateDirection(const UOrionInteractionTargetComponent* InteractionTarget) const
{
	/*if (!IsValid(InteractionTarget))
	{
		return false;
	}*/

	return true;
	/* Return True Early If Face Only Interaction is Not Required */
	/*if (!InteractionTarget->bOnlyFaceInteraction)
	{
		return true;
	}*/

	/* Get Interactor Direction from Our Location */
	/*FVector Direction = GetOwner()->GetActorLocation() - InteractionTarget->GetComponentLocation();
	Direction.Normalize();

	float DirectionPoint = 0.0f;

	return FVector::DotProduct(Direction, InteractionTarget->GetForwardVector()) > 0.5f;*/
}


void UOrionInteractionComponent::RegisterNewInteractionTarget(UOrionInteractionTargetComponent* NewInteractionTarget)
{
	/* Prevent Duplicate Registration */
	if (CurrentInteractionTarget == NewInteractionTarget)
	{
		return;
	}

	CurrentInteractionTarget = NewInteractionTarget;

	/* Local Interactor */
	if (IsLocalInteractor())
	{
		if (IsValid(CurrentInteractionTarget))
		{
			NewInteractionTarget->SetFocusState(true, GetOwningPawn());
		}

		if (OnInteractionChanged.IsBound())
		{
			OnInteractionChanged.Broadcast(NewInteractionTarget, NewInteractionTarget ? NewInteractionTarget->Tag : FGameplayTag().EmptyTag);
		}
	}
}

void UOrionInteractionComponent::DeRegisterInteractionTarget()
{
	/* Cancel Interaction If Already Interacting */
	if (bInteracting)
	{
		CancelInteraction();
	}

	/* Local Interactor */
	if (IsLocalInteractor())
	{
		if (IsValid(CurrentInteractionTarget))
		{
			CurrentInteractionTarget->SetFocusState(false, nullptr);
		}

		if (OnInteractionChanged.IsBound())
		{
			OnInteractionChanged.Broadcast(nullptr, FGameplayTag().EmptyTag);
		}
	}

	CurrentInteractionTarget = nullptr;
}

void UOrionInteractionComponent::UpdateInteractionTrace()
{
	if (bInteracting)
	{
		/* If Interacting Get the New Interaction Candidate and Compare to the Current Interacting Component*/
		const UOrionInteractionTargetComponent* NewInteraction = GetInteractionTrace();

		if (NewInteraction != CurrentInteractionTarget)
		{
			/* Cancel Interaction If not Valid Interaction */
			DeRegisterInteractionTarget();
		}

	}
	else if (IsLocalInteractor())
	{
		/* Locally Get Interaction and Validate the Component */
		UOrionInteractionTargetComponent* NewInteraction = GetInteractionTrace();

		/* Register If New Interaction is Not Equal to the Current Candidate */
		if (NewInteraction != CurrentInteractionTarget)
		{
			RegisterNewInteractionTarget(NewInteraction);
		}
		

	}

	OnInteractionUpdated.Broadcast(CurrentInteractionTarget, CurrentInteractionTarget ? CurrentInteractionTarget->Tag : FGameplayTag().EmptyTag);
	
	/*if (LastInteractionTarget && LastInteractionTarget->Mode == EOrionInteractionMode::Hold && LastInteractionTarget->IsInteractionInProgress() && CurrentInteractionTarget == nullptr)
	{
		LastInteractionTarget->EndInteraction(false);
	}*/
	

}

UOrionInteractionTargetComponent* UOrionInteractionComponent::GetInteractionTrace()
{
	if (!bIsInteractionEnabled)
	{
		return nullptr;
	}

	FVector TraceStart;
	FRotator Rotation;
	InteractionhitResults.Empty();
	TArray<AActor*> InteractionActors;

	FCollisionQueryParams CollisionQueryParams(SCENE_QUERY_STAT(AOrionInteractionComponent_UpdateInteracionTrace));
	CollisionQueryParams.AddIgnoredActor(GetOwningPawn());


	switch (InteractionType)
	{
		case EOrionInteractionType::E_InteractionPawn:
		{
			TraceStart = GetOwningPawn()->GetActorLocation();
			Rotation = GetOwningPawn()->GetActorRotation();
			TraceStart += GetOwningPawn()->GetActorForwardVector() * InteractionOffset.X;
			TraceStart += GetOwningPawn()->GetActorRightVector() * InteractionOffset.Y;
			TraceStart += GetOwningPawn()->GetActorUpVector() * InteractionOffset.Z;
			FCollisionShape Sphere = FCollisionShape::MakeSphere(InteractionRadius);

			TArray<FHitResult> HitResults;
			GetWorld()->SweepMultiByChannel(HitResults, TraceStart, TraceStart, FQuat::Identity, InteractionTraceChannel, Sphere);


			for (auto& Hit : HitResults)
			{
				if (!Hit.GetActor())
					continue;

				if (UOrionInteractionTargetComponent* TargetInteractable = Hit.GetActor()->FindComponentByClass<UOrionInteractionTargetComponent>())
				{
					if (Hit.GetActor()->IsPendingKillPending() || Hit.GetActor()->IsHidden())
						continue;

					InteractionhitResults.Add(Hit);
					InteractionActors.AddUnique(Hit.GetActor());
				}
			}

			if (InteractionIndex < InteractionhitResults.Num())
			{
				float Dist;
				return UGameplayStatics::FindNearestActor(GetOwningPawn()->GetActorLocation(), InteractionActors, Dist)->FindComponentByClass<UOrionInteractionTargetComponent>();
			}
	#if WITH_EDITOR
			if (bDebugTrace)
			{
				DrawDebugSphere(
					GetWorld(),
					TraceStart,
					InteractionRadius,
					12,
					FColor::Red,
					false,
					2.f,
					0,
					1.f
				);
			}
	#endif // WITH_EDITOR

			break;
		}

		case EOrionInteractionType::E_InteractionCamera:
		{
			OwningController->GetPlayerViewPoint(TraceStart, Rotation);

			const FVector TraceDirection = Rotation.Vector();
			FVector TraceEnd = TraceStart + TraceDirection * InteractionTraceDistance;

			FHitResult HitResult;
			GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, InteractionTraceChannel, CollisionQueryParams);
			InteractionhitResults.Add(HitResult);


			if ((InteractionhitResults.IsValidIndex(0)
				&& InteractionhitResults[0].bBlockingHit
				&& InteractionhitResults[0].GetActor()
				&& !InteractionhitResults[0].GetActor()->IsPendingKillPending())
				|| !InteractionhitResults[0].GetActor()->IsHidden())
			{
				if (UOrionInteractionTargetComponent* TargetInteractable = InteractionhitResults[0].GetActor()->FindComponentByClass<UOrionInteractionTargetComponent>())
				{
					return TargetInteractable;
				}

			}
	#if WITH_EDITOR
			if (bDebugTrace)
			{
				DrawDebugLine(
					GetWorld(),
					TraceStart,
					TraceEnd,
					FColor::Red,
					false,
					2.f,
					0,
					1.f
				);
			}
	#endif // WITH_EDITOR
			break;
		}
	}

	return nullptr;


	
}

void UOrionInteractionComponent::StartInteraction()
{
	/* Make Sure Interaction Starts on Authority */
	if (GetInteractorRole() != ROLE_Authority)
	{
		Server_StartInteraction();
		return;
	}

	/* Prevent New Interaction If One Already In Progress */
	if (bInteracting)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to Start Interaction Due to In Progress Interaction"));
		return;
	}

    /* Get Server Sided Interaction */
    CurrentInteractionTarget = GetInteractionTrace();

    if (!IsValid(CurrentInteractionTarget))
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to Start Interaction Due to No Interaction Available On Server Side"));
		return;
	}

	/* Start the Interaction */

	if (bIsInteractionEnabled &&
		CurrentInteractionTarget &&
		CurrentInteractionTarget->CanInteract())
	{
		if (CurrentInteractionTarget->DecoratorConditionMet())
		{
			SetInteracting(true);
			CurrentInteractionTarget->StartInteraction(GetOwningPawn());
			OnInteractionStarted.Broadcast(CurrentInteractionTarget, CurrentInteractionTarget ? CurrentInteractionTarget->Tag : FGameplayTag().EmptyTag);
		}
		else
		{
			OnInteractionBlocked.Broadcast(CurrentInteractionTarget, CurrentInteractionTarget ? CurrentInteractionTarget->Tag : FGameplayTag().EmptyTag);
		}
	}
	
	
}

void UOrionInteractionComponent::Server_StartInteraction_Implementation()
{
	StartInteraction();
}

void UOrionInteractionComponent::CancelInteraction()
{
	if (GetInteractorRole() != ROLE_Authority)
	{
		Server_CancelInteraction();
		return;
	}
	if (!bInteracting || !IsValid(CurrentInteractionTarget))
	{
		return;
	}

	SetInteracting(false);

	if (CurrentInteractionTarget &&
		CurrentInteractionTarget->Mode == EOrionInteractionMode::Hold &&
		CurrentInteractionTarget->IsInteractionInProgress())
	{
		CurrentInteractionTarget->EndInteraction(false);
		OnInteractionCanceled.Broadcast(CurrentInteractionTarget, CurrentInteractionTarget ? CurrentInteractionTarget->Tag : FGameplayTag().EmptyTag);
	}
}

void UOrionInteractionComponent::Server_CancelInteraction_Implementation()
{
	CancelInteraction();
}


void UOrionInteractionComponent::EndInteraction(UOrionInteractionTargetComponent* InteractionTarget, bool bCompleted)
{
	check(InteractionTarget);

	SetInteracting(false);

	if (bCompleted)
	{
		OnInteractionCompleted.Broadcast(InteractionTarget, InteractionTarget ? InteractionTarget->Tag : FGameplayTag().EmptyTag);
	}
	
}



