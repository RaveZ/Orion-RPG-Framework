// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "TargetingSystem/OrionTargetingSystemComponent.h"
#include "TargetingSystem/OrionTargetComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"  // for debug, optional
#include "Engine/OverlapResult.h"

UOrionTargetingSystemComponent::UOrionTargetingSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UOrionTargetingSystemComponent::BeginPlay()
{
	Super::BeginPlay();
	OwningPawn = GetOwningPawn();
	SetEnableTargeting(bTargetingEnabled);
}

void UOrionTargetingSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bTargetingEnabled)
		return;

	// 1. Validate current target
	CheckTargetLoss();

	// 2. Update camera if we have a target
	if (CurrentTarget && bEnableCameraControl)
	{
		UpdateTargetCamera(DeltaTime);
	}
}

APawn* UOrionTargetingSystemComponent::GetOwningPawn() const
{
	if (OwningPawn)
		return OwningPawn;

	if (APawn* Pawn = Cast<APawn>(GetOwner()))
		return Pawn;

	return nullptr;
}

float UOrionTargetingSystemComponent::GetDistanceToTarget() const
{
	if (!CurrentTarget || !OwningPawn)
		return 0.0f;

	return FVector::Dist(OwningPawn->GetActorLocation(), CurrentTarget->GetComponentLocation());
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
	else if (bAutoAcquireTargetOnEnable && !CurrentTarget)
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

void UOrionTargetingSystemComponent::CycleTarget(bool bNext)
{
	if (!bTargetingEnabled)
		return;

	TArray<UOrionTargetComponent*> ValidTargets = GetValidTargets();
	if (ValidTargets.Num() == 0)
	{
		ClearTarget();
		return;
	}

	// If no current target, acquire the best one
	if (!CurrentTarget)
	{
		SearchTarget();
		return;
	}

	int32 CurrentIndex = ValidTargets.IndexOfByKey(CurrentTarget);
	if (CurrentIndex == INDEX_NONE)
	{
		// Current target is no longer valid -> clear and optionally reacquire
		ClearTarget();
		if (bAutoCycleAfterTargetLost)
			SearchTarget();
		return;
	}

	int32 NewIndex = bNext ? CurrentIndex + 1 : CurrentIndex - 1;
	if (bWrapTargetCycle)
	{
		NewIndex = (NewIndex + ValidTargets.Num()) % ValidTargets.Num();
	}
	else
	{
		if (NewIndex < 0 || NewIndex >= ValidTargets.Num())
		{
			ClearTarget();
			return;
		}
	}

	UOrionTargetComponent* NewTarget = ValidTargets[NewIndex];
	if (NewTarget != CurrentTarget)
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
		OnLostTarget.Broadcast(CurrentTarget);
		CurrentTarget = nullptr;
		OnTargetCleared.Broadcast();
	}
}

void UOrionTargetingSystemComponent::ToggleTargetingWithSearch()
{
	if (!bTargetingEnabled)
	{
		SetEnableTargeting(true);
		SearchTarget(); 
	}
	else if (CurrentTarget)
	{
		ClearTarget();
	}
	else
	{
		SearchTarget();
	}
}

float UOrionTargetingSystemComponent::ScoreTarget(const UOrionTargetComponent* Target) const
{
	if (!Target || !OwningPawn)
		return BIG_NUMBER;

	float Distance = FVector::Dist(OwningPawn->GetActorLocation(), Target->GetComponentLocation());
	return Distance;
}

bool UOrionTargetingSystemComponent::IsTargetValid(UOrionTargetComponent* Target) const
{
	if (!Target || !Target->GetOwner() || !Target->GetOwner()->IsValidLowLevel())
		return false;

	if (!Target->IsTargetable())
		return false;

	// Distance check (including tolerance)
	float Distance = FVector::Dist(OwningPawn->GetActorLocation(), Target->GetComponentLocation());

	if (Distance > MaxDistance + LossDistanceTolerance)
		return false;

	return true;
}

TArray<UOrionTargetComponent*> UOrionTargetingSystemComponent::GetValidTargets() const
{
    TArray<UOrionTargetComponent*> Targets;
    if (!OwningPawn)
        return Targets;

    APlayerController* PC = Cast<APlayerController>(OwningPawn->GetController());
    if (!PC && bOnlyTargetsInCameraView)
        return Targets;

    // Sphere overlap
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(SearchRadius);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningPawn);

    GetWorld()->OverlapMultiByChannel(Overlaps, OwningPawn->GetActorLocation(), FQuat::Identity, TraceChannel, Sphere, QueryParams);

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* Actor = Overlap.GetActor();
        if (!Actor) continue;

        UOrionTargetComponent* TargetComp = Actor->FindComponentByClass<UOrionTargetComponent>();
        if (!TargetComp || !IsTargetValid(TargetComp))
            continue;

        if (bOnlyTargetsInCameraView && PC)
        {
            if (!IsTargetInCameraView(TargetComp, PC))
                continue;
        }

        Targets.Add(TargetComp);
    }
    return Targets;
}

// New helper function
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
		float MaxScreenDistance = FMath::Tan(FMath::DegreesToRadians(MaxAngleFromCenter)) * Center.X; // renamed variable
		if (DistanceFromCenter > MaxScreenDistance)
			return false;
	}

	return true;
}

bool UOrionTargetingSystemComponent::HasLineOfSight(const FVector& Start, const FVector& End) const
{
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwningPawn);
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
		if (bAutoCycleAfterTargetLost)
			SearchTarget();
	}
}

void UOrionTargetingSystemComponent::UpdateTargetCamera(float DeltaTime)
{
	if (!OwningPawn || !CurrentTarget)
		return;

	APlayerController* PC = Cast<APlayerController>(OwningPawn->GetController());
	if (!PC)
		return;

	FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
	FVector TargetLocation = CurrentTarget->GetComponentLocation();
    
	
	CameraLocation.Z += CameraVerticalOffset;
    
	FRotator DesiredRot = UKismetMathLibrary::FindLookAtRotation(CameraLocation, TargetLocation);

	ApplyControlRotation(DesiredRot, DeltaTime);
}

void UOrionTargetingSystemComponent::ApplyControlRotation(FRotator NewRotation, float DeltaTime)
{
	APlayerController* PC = Cast<APlayerController>(OwningPawn->GetController());
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