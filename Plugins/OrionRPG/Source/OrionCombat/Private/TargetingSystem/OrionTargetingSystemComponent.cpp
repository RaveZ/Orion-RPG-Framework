// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "TargetingSystem/OrionTargetingSystemComponent.h"


UOrionTargetingSystemComponent::UOrionTargetingSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	TraceChannel = ECollisionChannel::ECC_Pawn;
	bTargetingEnabled = true;
}


void UOrionTargetingSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningPawn = GetOwningPawn();
	MaxDistance = 1250.0f;
	SetEnableTargeting(true);
}


void UOrionTargetingSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                   FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MaxDistance)
}

APawn* UOrionTargetingSystemComponent::GetOwningPawn() const
{
	if (OwningPawn)
	{
		return OwningPawn;
	}
	else if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		return OwningPawn;
	}

	return nullptr;
}

void UOrionTargetingSystemComponent::SetEnableTargeting(bool bEnable)
{
	if (bEnable == bTargetingEnabled) return;
	bTargetingEnabled = bEnable;
}

void UOrionTargetingSystemComponent::SearchTarget()
{
}

