// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "AI/OrionAIController.h"

#include "Kismet/KismetMathLibrary.h"
#include "Components/StateTreeAIComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"

AOrionAIController::AOrionAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(
		TEXT("AIPerception")
	);
	StateTreeAIComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));
}

void AOrionAIController::BeginPlay()
{
	Super::BeginPlay();
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ThisClass::PerceptionUpdated);
	
}

void AOrionAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AOrionAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (InPawn)
	{
		OriginLocation = InPawn->GetActorLocation();
	}
}

void AOrionAIController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn)
{
	Super::UpdateControlRotation(DeltaTime, bUpdatePawn);
	if (TargetActor)
	{
		APawn* const MyPawn = GetPawn();
		const FRotator CurrentPawnRotation = MyPawn->GetActorRotation();

		SmoothTargetRotation = UKismetMathLibrary::RInterpTo_Constant(MyPawn->GetActorRotation(), ControlRotation, DeltaTime, SmoothFocusInterpSpeed);
		if (CurrentPawnRotation.Equals(SmoothTargetRotation, 1e-3f) == false)
		{
			//Change rotation using the Smooth Target Rotation
			MyPawn->FaceRotation(SmoothTargetRotation, DeltaTime);
		}
	}
}

void AOrionAIController::SetTarget(AActor* InTarget, const bool bSetFocus)
{
	TargetActor = InTarget;
	if (TargetActor && bSetFocus)
	{
		SetFocus(TargetActor);
		K2_OnTargetSet(TargetActor);
	}

	if (!TargetActor)
	{
		ClearFocus(EAIFocusPriority::Gameplay);
	}
}

void AOrionAIController::ClearTarget()
{
	SetTarget(nullptr, false); 
}

void AOrionAIController::PerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (Stimulus.WasSuccessfullySensed())
	{
		if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
		{
			const float Distance = FVector::Dist(Stimulus.ReceiverLocation, Stimulus.StimulusLocation);

		}
		else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
		{
		}
	}
}

void AOrionAIController::PauseLogic()
{
	check(StateTreeAIComponent);
	StateTreeAIComponent->PauseLogic("Pause Logic");
}

void AOrionAIController::ResumeLogic()
{
	check(StateTreeAIComponent);
	StateTreeAIComponent->ResumeLogic("Resume Logic");
}


