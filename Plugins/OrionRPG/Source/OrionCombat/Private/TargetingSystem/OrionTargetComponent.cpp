// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "TargetingSystem/OrionTargetComponent.h"

UOrionTargetComponent::UOrionTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // no need to tick unless you want
}

void UOrionTargetComponent::BeginPlay()
{
	Super::BeginPlay();
	OwningPawn = GetOwningPawn();
}

void UOrionTargetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

APawn* UOrionTargetComponent::GetOwningPawn() const
{
	if (OwningPawn)
		return OwningPawn;

	if (APawn* Pawn = Cast<APawn>(GetOwner()))
		return Pawn;

	return nullptr;
}