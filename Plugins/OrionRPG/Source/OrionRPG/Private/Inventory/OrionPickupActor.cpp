// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Inventory/OrionPickupActor.h"
#include "Engine/World.h"

AOrionPickupActor::AOrionPickupActor()
{
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(RootComponent);
	PickupMesh->SetSimulatePhysics(true);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PickupMesh->SetRenderCustomDepth(true);
	PickupMesh->SetCustomDepthStencilValue(255);

	InteractionTargetComp = CreateDefaultSubobject<UOrionInteractionTargetComponent_PickupItem>(TEXT("InteractionTargetComp - Pickup")); 
	InteractionTargetComp->SetupAttachment(PickupMesh);

}

void AOrionPickupActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AOrionPickupActor::BeginPlay()
{
	Super::BeginPlay();
}

UOrionInteractionTargetComponent_PickupItem::UOrionInteractionTargetComponent_PickupItem()
{
	
}

void UOrionInteractionTargetComponent_PickupItem::InitializePickups()
{
}
