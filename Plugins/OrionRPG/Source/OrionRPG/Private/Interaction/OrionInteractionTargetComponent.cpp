// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Interaction/OrionInteractionTargetComponent.h"
#include "Interaction/OrionInteractionFunctionLibrary.h"
#include "Interaction/OrionInteractionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"



UOrionInteractionTargetComponent::UOrionInteractionTargetComponent()
{
	Mode = EOrionInteractionMode::Tap;
	HoldDuration = 1.0f;
	CurrentInteractor = nullptr;
	CurrentProgress = 0.0f;
	bAutoActivate = true;
	//SetComponentTickEnabled(false);
}

void UOrionInteractionTargetComponent::BeginPlay()
{
	Super::BeginPlay();
	
	bNativeCanInteract = true;
	AActor* Owner = GetOwner();
	if (Owner)
	{
		Owner->Tags.AddUnique(Tag.GetTagName());
	}
}



bool UOrionInteractionTargetComponent::CanInteract() const
{
	return bNativeCanInteract && (Mode != EOrionInteractionMode::None) && (CurrentInteractor == nullptr) && FMath::IsNearlyEqual(CurrentProgress, 0.0f, UE_KINDA_SMALL_NUMBER);
}

bool UOrionInteractionTargetComponent::DecoratorConditionMet() const
{
	int SumConditionMet = 0;
	//Evaluate Decorator
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;

	for (auto& Condition : InteractConditions)
	{
		if (Condition && PC && Pawn)
		{
			bool bConditionMet = Condition && Condition->InvertCondition ? !Condition->PerformConditionCheck(PC, Pawn) : Condition->PerformConditionCheck(PC, Pawn);
			if (bConditionMet)
			{
				SumConditionMet++;
			}
		}
		
	}
	return SumConditionMet >= InteractConditions.Num();
}

void UOrionInteractionTargetComponent::Focused(AActor* Interactor)
{
	// Blueprint event
	if (OnFocused.IsBound())
	{
		OnFocused.Broadcast(Interactor, this);
	}
}


void UOrionInteractionTargetComponent::StartInteraction(AActor* Interactor)
{
	//UE_LOG(LogTemp, Log, __FUNCTIONW__);
	check(Mode != EOrionInteractionMode::None);

	CurrentInteractor = Interactor;
	CurrentProgress = 0.0f;
	CurrentHoldTime= 0.0f;
	
	// Blueprint event
	if (OnStarted.IsBound())
	{
		OnStarted.Broadcast(CurrentInteractor, this);
	}

	if (Mode == EOrionInteractionMode::Tap)
	{
		if (OnProcessed.IsBound())
		{
			OnProcessed.Broadcast(CurrentInteractor, this);
		}
		else
		{
			EndInteraction(true);
		}
	}
	else if (Mode == EOrionInteractionMode::Hold)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_UpdateProgressValue, this, &UOrionInteractionTargetComponent::UpdateProgressValue, 0.0333f, true);
	}
}


void UOrionInteractionTargetComponent::EndInteraction(bool bCompleted)
{
	//UE_LOG(LogTemp, Log, __FUNCTIONW__);

	check(Mode != EOrionInteractionMode::None);

	// Blueprint event
	if (bCompleted)
	{
		UE_LOG(LogTemp, Log, TEXT("Interaction COMPLETED! (Actor: %s, Interactor: %s)"), *GetName(), *CurrentInteractor->GetName());

		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		APawn* Pawn = PC ? PC->GetPawn() : nullptr;
		for (auto& Event : Events)
		{
			if (Event)
			{
				if (PC && Pawn)
				{
					Event->BeginEvent(PC, Pawn);
				}
			}
		}
		if (OnCompleted.IsBound())
		{
			OnCompleted.Broadcast(CurrentInteractor, this);
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Interaction CANCELED! (Actor: %s, Interactor: %s)"), *GetName(), *CurrentInteractor->GetName());

		if (OnCanceled.IsBound())
		{
			OnCanceled.Broadcast(CurrentInteractor, this);
		}
	}

	if (UOrionInteractionComponent* InteractionComp = UOrionInteractionFunctionLibrary::GetInteractionComponent(GetWorld()))
	{
		InteractionComp->InteractionEnded(this, bCompleted);
	}

	bNativeCanInteract = false;
	FTimerDelegate ResetInteractionTimerDel;
	TWeakObjectPtr<UOrionInteractionTargetComponent> WeakThis = this;
	ResetInteractionTimerDel.BindLambda([this, WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				bNativeCanInteract = true;
			}
		});

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_ResetInteraction, 
		ResetInteractionTimerDel,
		0.5f,
		false 
	);

	CurrentInteractor = nullptr;
	CurrentProgress = 0.0f;
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_UpdateProgressValue);
}


bool UOrionInteractionTargetComponent::IsInteractionInProgress() const
{
	return (CurrentInteractor != nullptr) || (CurrentProgress > 0.0f && CurrentProgress < 1.0f);
}


void UOrionInteractionTargetComponent::UpdateInteractionProgress(float Progress)
{
	CurrentProgress = Progress;
}


void UOrionInteractionTargetComponent::UpdateProgressValue()
{
	CurrentHoldTime = FMath::Clamp(CurrentHoldTime + 0.0333f, 0.0f, HoldDuration);
	CurrentProgress = FMath::Clamp(CurrentHoldTime / HoldDuration, 0.f, 1.f);
	if (CurrentProgress >= 1.f)
	{
		CurrentProgress = 0.0f;
		CurrentHoldTime= 0.0f;
		EndInteraction(true);
	}
}
