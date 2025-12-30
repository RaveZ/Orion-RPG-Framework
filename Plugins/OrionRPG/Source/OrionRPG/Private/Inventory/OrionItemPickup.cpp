// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Inventory/OrionItemPickup.h"
#include "Engine/World.h"

/**
 * Orion Lootable Actor
 */
AOrionLootable::AOrionLootable()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetSimulatePhysics(true);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetRenderCustomDepth(true);
	Mesh->SetCustomDepthStencilValue(255);

	InteractionTargetComp = CreateDefaultSubobject<UOrionInteractionTargetComponent_LootItem>(TEXT("InteractionTargetComp - Loot Item"));
	InteractionTargetComp->SetupAttachment(Mesh);

}

void AOrionLootable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AOrionLootable::BeginPlay()
{
	Super::BeginPlay();
}

/**
 * Interaction target component - loot item
 */
UOrionInteractionTargetComponent_LootItem::UOrionInteractionTargetComponent_LootItem()
{
}


/**
 * Orion Item Pickup Actor
 */
AOrionItemPickup::AOrionItemPickup()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetSimulatePhysics(true);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetRenderCustomDepth(true);
	Mesh->SetCustomDepthStencilValue(255);

	InteractionTargetComp = CreateDefaultSubobject<UOrionInteractionTargetComponent_PickupItem>(TEXT("InteractionTargetComp - Pickup Item"));
	InteractionTargetComp->SetupAttachment(Mesh);
}

void AOrionItemPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AOrionItemPickup::BeginPlay()
{
	Super::BeginPlay();
}

/**
 * Interaction target component - pickup item
 */
UOrionInteractionTargetComponent_PickupItem::UOrionInteractionTargetComponent_PickupItem()
{
	
}


