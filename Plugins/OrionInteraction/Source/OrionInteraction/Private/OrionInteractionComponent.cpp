#include "OrionInteractionComponent.h"
#include "OrionInteractionTargetComponent.h"
#include "CollisionQueryParams.h"
#include "GameFramework/PlayerController.h"




UOrionInteractionComponent::UOrionInteractionComponent()
{
	// Initialize variables
	InteractionTraceChannel = ECC_WorldDynamic;
	InteractionType = EOrionInteractionType::E_InteractionPawn;
	InteractionRadius = 250.f;
	InteractionTraceDistance = 300.0f;
	InteractionTraceIntervalSecond = 0.3f;
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
			CurrentInteractable = nullptr;

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

	FCollisionQueryParams CollisionQueryParams(SCENE_QUERY_STAT(AOrionInteractionComponent_UpdateInteracionTrace));
	CollisionQueryParams.AddIgnoredActor(GetOwningPawn());


	switch (InteractionType)
	{
		case EOrionInteractionType::E_InteractionPawn:
		{
			TraceStart = GetOwningPawn()->GetActorLocation();
			Rotation = GetOwningPawn()->GetActorRotation();
			TraceStart += InteractionOffset;
			FCollisionShape Sphere = FCollisionShape::MakeSphere(InteractionRadius);

			TArray<FHitResult> HitResults;
			GetWorld()->SweepMultiByChannel(HitResults, TraceStart, TraceStart, FQuat::Identity, InteractionTraceChannel, Sphere);

			PrevInteractable = CurrentInteractable;
			CurrentInteractable = nullptr;

			for (auto& Hit : HitResults)
			{
				if (UOrionInteractionTargetComponent* TargetInteractable = Hit.GetActor()->FindComponentByClass<UOrionInteractionTargetComponent>())
				{
					InteractionhitResults.Add(Hit);
				}
			}

			if (InteractionIndex < InteractionhitResults.Num())
			{
				CurrentInteractable = InteractionhitResults[InteractionIndex].GetActor()->FindComponentByClass<UOrionInteractionTargetComponent>();
			}

			if (bDebugTrace)
			{
				DrawDebugSphere(
					GetWorld(),
					TraceStart,
					InteractionRadius,
					12, // Number of segments
					FColor::Red,
					false, // Persistent lines
					2.f, // Duration
					0, // Depth priority
					1.f // Line thickness
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

			PrevInteractable = CurrentInteractable;
			CurrentInteractable = nullptr;

			if (InteractionhitResults[0].bBlockingHit && InteractionhitResults[0].GetComponent())
			{
				CurrentInteractable = InteractionhitResults[0].GetActor()->FindComponentByClass<UOrionInteractionTargetComponent>();
			}
			if (bDebugTrace)
			{
				DrawDebugLine(
					GetWorld(),
					TraceStart,
					TraceEnd,
					FColor::Red,
					false, // Persistent lines  
					2.f,  // Duration  
					0,    // Depth priority  
					1.f   // Line thickness  
				);
			}
			break;
		}
	}
	// If we have previous interactable and we lose focus from it, cancel interaction
	if (PrevInteractable && PrevInteractable->Mode == EOrionInteractionMode::Hold && PrevInteractable->IsInteractionInProgress() && CurrentInteractable == nullptr)
	{
		PrevInteractable->EndInteraction(false);
	}

}

void UOrionInteractionComponent::StartInteraction()
{
	if (bIsInteractionEnabled &&
		CurrentInteractable &&
		CurrentInteractable->CanInteract())
	{
		CurrentInteractable->StartInteraction(GetOwningPawn());
	}
}

void UOrionInteractionComponent::CancelInteraction()
{
	if (CurrentInteractable &&
		CurrentInteractable->Mode == EOrionInteractionMode::Hold &&
		CurrentInteractable->IsInteractionInProgress())
	{
		CurrentInteractable->EndInteraction(false);
	}
}
