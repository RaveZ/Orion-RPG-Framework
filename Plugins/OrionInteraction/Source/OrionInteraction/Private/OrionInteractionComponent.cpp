#include "OrionInteractionComponent.h"
#include "OrionInteractionTargetComponent.h"
#include "CollisionQueryParams.h"
#include "GameFramework/PlayerController.h"

UOrionInteractionComponent::UOrionInteractionComponent()
{
	// Initialize variables
	InteractionTraceDistance = 300.0f;
	InteractionTraceIntervalSecond = 0.3f;
	bIsInteractionEnabled = false;
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

	// Find interactable
	FVector TraceStart;
	FRotator Rotation;
	OwningController->GetPlayerViewPoint(TraceStart, Rotation);

	const FVector TraceDirection = Rotation.Vector();
	FVector TraceEnd = TraceStart + TraceDirection * InteractionTraceDistance;

	FCollisionQueryParams CollisionQueryParams(SCENE_QUERY_STAT(AOrionInteractionComponent_UpdateInteracionTrace));
	CollisionQueryParams.AddIgnoredActor(GetOwningPawn());

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldDynamic, CollisionQueryParams);

	UOrionInteractionTargetComponent* PrevInteractable = CurrentInteractable;
	CurrentInteractable = nullptr;

	if (HitResult.bBlockingHit && HitResult.GetComponent())
	{
		CurrentInteractable = Cast<UOrionInteractionTargetComponent>(HitResult.GetComponent());
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
