// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "TargetingSystem/OrionTargetComponent.h"

#include "TargetingSystem/OrionTargetingSystemComponent.h"

UOrionTargetComponent::UOrionTargetComponent()
{
	
}

void UOrionTargetComponent::BeginPlay()
{
	Super::BeginPlay();
	OwningPawn = GetOwningPawn();
}

void UOrionTargetComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

APawn* UOrionTargetComponent::GetOwningPawn() const
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
