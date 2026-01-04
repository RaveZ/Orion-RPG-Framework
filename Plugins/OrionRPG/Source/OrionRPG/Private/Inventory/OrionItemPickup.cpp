// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Inventory/OrionItemPickup.h"
#include "Inventory/OrionInventoryFunctionLibrary.h"
#include "Inventory/OrionInventoryGraph.h"
#include "Inventory/OrionInventoryNode_ItemDefinition.h"
#include "Engine/World.h"

/**
 * Orion Lootable Actor
 */
AOrionLootable::AOrionLootable()
{
	bReplicates = true;
	SetReplicateMovement(true);

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
	ActionDisplayText = FText::FromString("Loot");
}

void UOrionInteractionTargetComponent_LootItem::BeginPlay()
{
	Super::BeginPlay();
}


void UOrionInteractionTargetComponent_LootItem::AddItemToContainer(FOrionPickupDefinition& InPickupDefinition)
{
	ContainedItems.Add(InPickupDefinition);
}

void UOrionInteractionTargetComponent_LootItem::OnLocalCompleted_Implementation(AActor* Interactor, UOrionInteractionTargetComponent* Component)
{
	Super::OnLocalCompleted_Implementation(Interactor, Component);
	UOrionInventoryFunctionLibrary::LootItemFromActor(this, GetOwner(), Interactor);
}



/**
 * Orion Item Pickup Actor
 */
AOrionItemPickup::AOrionItemPickup()
{
	bReplicates = true;
	SetReplicateMovement(true);

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
	ActionDisplayText = FText::FromString("Pickup");
}

void UOrionInteractionTargetComponent_PickupItem::BeginPlay()
{
	Super::BeginPlay();
}

void UOrionInteractionTargetComponent_PickupItem::SetItem(FOrionPickupDefinition& InPickupDefinition)
{
	UOrionInventoryGraph* InventoryDB = UOrionInventoryFunctionLibrary::GetInventoryDB(this);
	
	UOrionInventoryNode_ItemDefinition* ItemDef = InventoryDB ? InventoryDB->ItemDefinitions.FindRef(InPickupDefinition.ItemTag) : nullptr;
	if(ItemDef)
	{
		ActionDisplayText = FText::FromString("Pickup " + ItemDef->DisplayName.ToString());
	}
	ContainedItem = InPickupDefinition;	
}





void UOrionInteractionTargetComponent_PickupItem::OnLocalCompleted_Implementation(AActor* Interactor, UOrionInteractionTargetComponent* Component)
{
	Super::OnLocalCompleted_Implementation(Interactor, Component);
	int32 Remainder;
	FItemInstanceData ItemData;
	UOrionInventoryFunctionLibrary::AddInventoryItem(this, Remainder, ItemData, Interactor, ContainedItem.ItemTag, ContainedItem.Count);

	if(Remainder <= 0)
	{
		AActor* OwnerActor = GetOwner();
		if(OwnerActor)
		{
			OwnerActor->Destroy();
		}
	}
	else
	{
		ContainedItem.Count = Remainder;
	}
}