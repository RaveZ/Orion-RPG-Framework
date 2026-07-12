// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "TargetingSystem/OrionTargetingSystemComponent.h"
#include "TargetingSystem/OrionTargetComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"  // for debug, optional
#include "OrionCombatFunctionLibrary.h"
#include "Engine/OverlapResult.h"

UOrionTargetingSystemComponent::UOrionTargetingSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	bOnlyTargetsInCameraView = true;
	
	MaxDistance = 1250.0f;
	SearchRadius = 1500.0f;
	CameraVerticalOffset = 30.0f;
	MaxAngleFromCenter = 45.0f;
	CameraBlendSpeed = 540.0f;
	
	TraceChannel = ECC_Pawn;
}

void UOrionTargetingSystemComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Cache the owning controller
	OwningController = GetOwningController();

	if (OwningController)
	{
		// Fix: Use a delegate to bind the function properly
		OwningController->OnPossessedPawnChanged.AddDynamic(this, &UOrionTargetingSystemComponent::OnPossessedPawnChanged);
     
	}
	SetEnableTargeting(true);
}

void UOrionTargetingSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bTargetingEnabled)
		return;

	
	static int32 TickCounter = 0;
	if (++TickCounter % 20 == 0) 
	{
		CheckTargetLoss();
		if (CurrentTarget && !UOrionCombatFunctionLibrary::IsAlive(GetOwningPawn()))
		{
			ClearTarget();
		}
		if (CurrentTarget && !UOrionCombatFunctionLibrary::IsAlive(CurrentTarget->GetOwner()))
		{
			ClearTarget();
			CycleToNextTarget();
		}
	}

	
	if (CurrentTarget)
	{
		UpdateTargetCamera(DeltaTime);
	}
}

APawn* UOrionTargetingSystemComponent::GetOwningPawn() const
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

APlayerController* UOrionTargetingSystemComponent::GetOwningController() const
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

void UOrionTargetingSystemComponent::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (NewPawn == nullptr)
	{
		SetEnableTargeting(false);
	}
	else
	{
		SetEnableTargeting(true);
	}
}

float UOrionTargetingSystemComponent::GetDistanceToTarget() const
{
	if (!CurrentTarget || !GetOwningPawn())
		return 0.0f;

	return FVector::Dist(GetOwningPawn()->GetActorLocation(), CurrentTarget->GetComponentLocation());
}

void UOrionTargetingSystemComponent::SetEnableTargeting(bool bEnable)
{
	if (bEnable == bTargetingEnabled)
		return;

	bTargetingEnabled = bEnable;
	OnTargetingEnabledChanged.Broadcast(bTargetingEnabled);

	if (!bTargetingEnabled)
	{
		ClearTarget();
	}
	else if (!CurrentTarget)
	{
		SearchTarget();
	}
}

void UOrionTargetingSystemComponent::SearchTarget()
{
	TArray<UOrionTargetComponent*> ValidTargets = GetValidTargets();
	if (ValidTargets.Num() == 0)
	{
		ClearTarget();
		return;
	}

	// Sort by score (lower is better)
	ValidTargets.Sort([this](const UOrionTargetComponent& A, const UOrionTargetComponent& B)
	{
		return ScoreTarget(&A) < ScoreTarget(&B);
	});

	UOrionTargetComponent* BestTarget = ValidTargets[0];
	if (BestTarget != CurrentTarget)
	{
		CurrentTarget = BestTarget;
		OnTargetChanged.Broadcast(CurrentTarget);
		OnTargetAcquired.Broadcast(CurrentTarget);
	}
}


void UOrionTargetingSystemComponent::CycleToNextTarget()
{
    if (!bTargetingEnabled)
        return;

	ValidTargetList = GetValidTargets();
	const int32 NumTargets = ValidTargetList.Num();
	
	const bool bShouldClear =
	 NumTargets == 0 ||
	 (NumTargets == 1 && CurrentTarget) ||
	 (NumTargets > 1 && CurrentCycleIndex == NumTargets - 1);

	if (bShouldClear)
	{
		ClearTarget();
		return;
	}
	
    // If we have no current target, acquire the best one
    if (!CurrentTarget)
    {
        SearchTarget();
        return;
    }

    // Find current target in the list
    int32 CurrentTargetIndex = ValidTargetList.IndexOfByKey(CurrentTarget);
    
    // If current target is not in valid list (maybe died or moved out of view)
    if (CurrentTargetIndex == INDEX_NONE)
    {
        ClearTarget();
        return;
    }

    // Calculate next index
    int32 NextIndex = (CurrentTargetIndex + 1) % (ValidTargetList.Num());
	CurrentCycleIndex += 1;
    
    if (NextIndex >= ValidTargetList.Num())
    {
        ClearTarget();  // Go back to not targeting anything
        return;
    }

    // Acquire the next target
    UOrionTargetComponent* NewTarget = ValidTargetList[NextIndex];
    if (NewTarget && NewTarget != CurrentTarget)
    {
        CurrentTarget = NewTarget;
        OnTargetChanged.Broadcast(CurrentTarget);
        OnTargetAcquired.Broadcast(CurrentTarget);
    }
}

void UOrionTargetingSystemComponent::ClearTarget()
{
	if (CurrentTarget)
	{
		CurrentCycleIndex = 0;
		OnLostTarget.Broadcast(CurrentTarget);
		CurrentTarget = nullptr;
		OnTargetCleared.Broadcast();
	}
}



float UOrionTargetingSystemComponent::ScoreTarget(const UOrionTargetComponent* Target) const
{
	if (!Target || !GetOwningPawn())
		return BIG_NUMBER;

	float Distance = FVector::Dist(GetOwningPawn()->GetActorLocation(), Target->GetComponentLocation());
	return Distance;
}

bool UOrionTargetingSystemComponent::IsTargetValid(UOrionTargetComponent* Target) const
{
	if (!Target || !Target->GetOwner() || !Target->GetOwner()->IsValidLowLevel())
		return false;

	if (!Target->IsTargetable())
		return false;

	// Distance check (including tolerance)
	float Distance = FVector::Dist(GetOwningPawn()->GetActorLocation(), Target->GetComponentLocation());

	if (Distance > MaxDistance)
		return false;

	return true;
}

TArray<UOrionTargetComponent*> UOrionTargetingSystemComponent::GetValidTargets() const
{
    TArray<UOrionTargetComponent*> Targets;
    if (!GetOwningPawn())
        return Targets;

    APlayerController* PC = Cast<APlayerController>(GetOwningPawn()->GetController());
    if (!PC && bOnlyTargetsInCameraView)
        return Targets;

    // Sphere overlap
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(SearchRadius);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwningPawn());

    GetWorld()->OverlapMultiByChannel(Overlaps, GetOwningPawn()->GetActorLocation(), FQuat::Identity, TraceChannel, Sphere, QueryParams);

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* Actor = Overlap.GetActor();
        if (!Actor) continue;
    	
    	if (!UOrionCombatFunctionLibrary::IsAlive(Actor)) continue;

        UOrionTargetComponent* TargetComp = Actor->FindComponentByClass<UOrionTargetComponent>();
        if (!TargetComp || !IsTargetValid(TargetComp))
            continue;

        if (bOnlyTargetsInCameraView && PC)
        {
            if (!IsTargetInCameraView(TargetComp, PC))
                continue;
        }

        Targets.AddUnique(TargetComp);
    }
    return Targets;
}

UOrionTargetComponent* UOrionTargetingSystemComponent::GetNearestValidTarget() const
{
	if (!GetOwningPawn())
		return nullptr;

	APlayerController* PC = Cast<APlayerController>(GetOwningPawn()->GetController());
	if (!PC && bOnlyTargetsInCameraView)
		return nullptr;

	TArray<UOrionTargetComponent*> ValidTargets = GetValidTargets();
    
	if (ValidTargets.Num() == 0)
		return nullptr;

	// Find the nearest target by distance
	UOrionTargetComponent* NearestTarget = nullptr;
	float NearestDistance = MAX_FLT;
	FVector PawnLocation = GetOwningPawn()->GetActorLocation();

	for (UOrionTargetComponent* Target : ValidTargets)
	{
		if (!Target)
			continue;
		float Distance = FVector::Dist(PawnLocation, Target->GetComponentLocation());

		if (Distance < NearestDistance)
		{
			NearestDistance = Distance;
			NearestTarget = Target;
		}
	}

	return NearestTarget;
	
}

bool UOrionTargetingSystemComponent::IsTargetInCameraView(UOrionTargetComponent* Target, APlayerController* PC) const
{
	if (!PC || !Target) return false;

	FVector TargetLocation = Target->GetComponentLocation();
	FVector2D ScreenPos;
	bool bProjected = PC->ProjectWorldLocationToScreen(TargetLocation, ScreenPos, true);
	if (!bProjected)
		return false; // behind camera

	int32 ViewportSizeX, ViewportSizeY;
	PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
	if (ViewportSizeX == 0 || ViewportSizeY == 0)
		return false;

	// Check if on screen
	if (ScreenPos.X < 0 || ScreenPos.X > ViewportSizeX || ScreenPos.Y < 0 || ScreenPos.Y > ViewportSizeY)
		return false;

	// Optional angle from center
	if (MaxAngleFromCenter > 0.0f)
	{
		FVector2D Center(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f);
		float DistanceFromCenter = FVector2D::Distance(ScreenPos, Center);
		float MaxScreenDistance = FMath::Tan(FMath::DegreesToRadians(MaxAngleFromCenter)) * Center.X;
		if (DistanceFromCenter > MaxScreenDistance)
			return false;
	}

	return true;
}

bool UOrionTargetingSystemComponent::HasLineOfSight(const FVector& Start, const FVector& End) const
{
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwningPawn());
	if (CurrentTarget && CurrentTarget->GetOwner())
		Params.AddIgnoredActor(CurrentTarget->GetOwner());

	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, TraceChannel, Params);
	return !Hit.bBlockingHit || Hit.GetActor() == (CurrentTarget ? CurrentTarget->GetOwner() : nullptr);
}

void UOrionTargetingSystemComponent::CheckTargetLoss()
{
	if (!CurrentTarget)
		return;

	if (!IsTargetValid(CurrentTarget))
	{
		ClearTarget();
	}
}

void UOrionTargetingSystemComponent::UpdateTargetCamera(float DeltaTime)
{
	if (!GetOwningPawn() || !CurrentTarget)
		return;

	APlayerController* PC = GetOwningController();
	if (!PC)
		return;

	APawn* Pawn = GetOwningPawn();
	if (!Pawn) return;
	
	FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
	FVector TargetLocation = CurrentTarget->GetComponentLocation();
	FVector PawnLocation = Pawn->GetActorLocation();
	
	float Distance = FVector::Dist(PawnLocation, TargetLocation);
	float AdjustedOffset = CameraVerticalOffset;
	
	
	float DistanceAlpha = FMath::GetMappedRangeValueClamped(
		FVector2D(300.0f, 1500.0f),
		FVector2D(0.0f, 1.0f),
		Distance
	);
	DistanceAlpha = FMath::Clamp(DistanceAlpha, 0.0f, 1.0f);
	
	float MaxZ = FMath::Lerp(0.0f, 200.0f, DistanceAlpha);
	
	AdjustedOffset += FMath::Clamp((Distance * 0.5f), 0.0f, MaxZ); // Slightly higher when far
	
	CameraLocation.Z += AdjustedOffset;
    
	FRotator DesiredRot = UKismetMathLibrary::FindLookAtRotation(CameraLocation, TargetLocation);

	if (Distance < 100.0f)
	{
		FRotator CurrentRot = PC->GetControlRotation();
		DesiredRot.Pitch = CurrentRot.Pitch;  
	}

	ApplyControlRotation(DesiredRot, DeltaTime);
}

void UOrionTargetingSystemComponent::ApplyControlRotation(FRotator NewRotation, float DeltaTime)
{
	APlayerController* PC = Cast<APlayerController>(GetOwningPawn()->GetController());
	if (!PC)
		return;

	if (CameraBlendSpeed <= 0.0f || DeltaTime <= 0.0f)
	{
		PC->SetControlRotation(NewRotation);
	}
	else
	{
		FRotator Current = PC->GetControlRotation();
		FRotator DeltaRot = UKismetMathLibrary::RInterpTo(Current, NewRotation, DeltaTime, CameraBlendSpeed);
		PC->SetControlRotation(DeltaRot);
	}
}