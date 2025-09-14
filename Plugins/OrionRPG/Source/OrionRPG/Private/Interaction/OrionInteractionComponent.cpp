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
	InteractionTraceChannel = ECC_WorldDynamic;
	InteractionType = EOrionInteractionType::E_InteractionPawn;
	InteractionRadius = 150.f;
	InteractionTraceDistance = 750.0f;
	InteractionTraceIntervalSecond = 0.3f;
	InteractionOffset = FVector(75.f, 0.f, 0.f);
	bIsInteractionEnabled = false;
	bDebugTrace = false;
}

void UOrionInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache the owning controller
	OwningController = GetOwningController();
	if (OwningController)
	{
        // Fix: Use a delegate to bind the function properly
        OwningController->OnPossessedPawnChanged.AddDynamic(this, &UOrionInteractionComponent::OnPossessedPawnChanged);
     
	}
	SetInteractionEnabled(true);
}
void UOrionInteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UOrionInteractionComponent, CurrentInteractionTarget);
	DOREPLIFETIME(UOrionInteractionComponent, bIsInteractionEnabled);
	DOREPLIFETIME(UOrionInteractionComponent, InteractionType);
	DOREPLIFETIME(UOrionInteractionComponent, InteractionIndex);
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

			World->GetTimerManager().SetTimer(TimerHandle_UpdateInteractionTrace, this, &UOrionInteractionComponent::UpdateInteractionTrace, InteractionTraceIntervalSecond, true);
		}
		else
		{
			World->GetTimerManager().ClearTimer(TimerHandle_UpdateInteractionTrace);
			CancelInteraction();
			CurrentInteractionTarget = nullptr;

		}
	}
}

void UOrionInteractionComponent::UpdateInteractionTrace()
{
	if (!bIsInteractionEnabled || OwningController == nullptr)
	{
		return;
	}

	UOrionInteractionTargetComponent* PrevInteractable = nullptr;
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

			PrevInteractable = CurrentInteractionTarget;
			CurrentInteractionTarget = nullptr;

			for (auto& Hit : HitResults)
			{
				if(!Hit.GetActor())
					continue;

				if (UOrionInteractionTargetComponent* TargetInteractable = Hit.GetActor()->FindComponentByClass<UOrionInteractionTargetComponent>())
				{
					if (Hit.GetActor()->IsPendingKillPending())
						continue;

					InteractionhitResults.Add(Hit);
					InteractionActors.AddUnique(Hit.GetActor());
				}
			}

			if (InteractionIndex < InteractionhitResults.Num())
			{
				float Dist;
				CurrentInteractionTarget = UGameplayStatics::FindNearestActor(GetOwningPawn()->GetActorLocation(), InteractionActors, Dist)->FindComponentByClass<UOrionInteractionTargetComponent>();
			}

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

			PrevInteractable = CurrentInteractionTarget;
			CurrentInteractionTarget = nullptr;

			if (InteractionhitResults[0].bBlockingHit 
				&& InteractionhitResults[0].GetComponent()
				&& !InteractionhitResults[0].GetActor()->IsPendingKillPending())
			{
				if (UOrionInteractionTargetComponent* TargetInteractable = InteractionhitResults[0].GetActor()->FindComponentByClass<UOrionInteractionTargetComponent>())
				{
					CurrentInteractionTarget = TargetInteractable;
				}
				
			}
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
			break;
		}
	}
	MulticastOnInteractionUpdated(CurrentInteractionTarget);
	// If we have previous interactable and we lose focus from it, cancel interaction
	if (PrevInteractable && PrevInteractable->Mode == EOrionInteractionMode::Hold && PrevInteractable->IsInteractionInProgress() && CurrentInteractionTarget == nullptr)
	{
		PrevInteractable->EndInteraction(false);
	}

}

void UOrionInteractionComponent::StartInteraction()
{
	if (bIsInteractionEnabled &&
		CurrentInteractionTarget &&
		CurrentInteractionTarget->CanInteract())
	{
		if (CurrentInteractionTarget->DecoratorConditionMet())
		{
			CurrentInteractionTarget->StartInteraction(GetOwningPawn());
			MulticastOnInteractionStarted(CurrentInteractionTarget);
		}
		else
		{
			MulticastOnInteractionBlocked(CurrentInteractionTarget);
		}
	}
}

void UOrionInteractionComponent::CancelInteraction()
{
	if (CurrentInteractionTarget &&
		CurrentInteractionTarget->Mode == EOrionInteractionMode::Hold &&
		CurrentInteractionTarget->IsInteractionInProgress())
	{
		CurrentInteractionTarget->EndInteraction(false);
		MulticastOnInteractionCanceled(CurrentInteractionTarget);
	}
}

void UOrionInteractionComponent::InteractionEnded(UOrionInteractionTargetComponent* Component, bool bCompleted)
{
	check(Component);

	if (bCompleted)
	{
		MulticastOnInteractionCompleted(Component);
	}
}

void UOrionInteractionComponent::MulticastOnInteractionStarted_Implementation(UOrionInteractionTargetComponent* Target)
{
	OnInteractionStarted.Broadcast(Target);
}

void UOrionInteractionComponent::MulticastOnInteractionCanceled_Implementation(UOrionInteractionTargetComponent* Target)
{
	OnInteractionCanceled.Broadcast(Target);
}

void UOrionInteractionComponent::MulticastOnInteractionCompleted_Implementation(UOrionInteractionTargetComponent* Target)
{
	OnInteractionCompleted.Broadcast(Target);
}

void UOrionInteractionComponent::MulticastOnInteractionUpdated_Implementation(UOrionInteractionTargetComponent* Target)
{
	OnInteractionUpdated.Broadcast(Target);
}

void UOrionInteractionComponent::MulticastOnInteractionBlocked_Implementation(UOrionInteractionTargetComponent* Target)
{
	OnInteractionBlocked.Broadcast(Target);
}
