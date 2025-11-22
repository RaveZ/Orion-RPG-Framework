// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Inventory/OrionActorViewer.h"
#include "Engine/World.h"

AOrionActorViewer::AOrionActorViewer()
{
	MinFOV = 25.f;
	MaxFOV = 50.f;
	
}

void AOrionActorViewer::BeginPlay()
{
	Super::BeginPlay();
	CurrentFOV = MinFOV;

}

void AOrionActorViewer::Zoom_Implementation(float Delta)
{
}

void AOrionActorViewer::Rotate_Implementation(float DeltaX, float DeltaY, float Sensitivity)
{
}
