// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Inventory/OrionItemPickup.h"
#include "OrionFunctionLibrary.h"
#include "Interaction/OrionInteractionTargetComponent.h"
#include "Components/StaticMeshComponent.h"
#include "OrionSaveGame.h"
#include "Inventory/OrionInventoryFunctionLibrary.h"
#include "Inventory/OrionInventoryGraph.h"
#include "Inventory/OrionInventoryNode_ItemDefinition.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

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

void AOrionLootable::OnPreSave_Implementation()
{
	ContainedItems = InteractionTargetComp->ContainedItems;
}

void AOrionLootable::OnSaved_Implementation()
{
}

void AOrionLootable::OnLoaded_Implementation()
{
	InteractionTargetComp->ContainedItems = ContainedItems;

	InteractionTargetComp->EvaluateItemList();
}

/**
 * Interaction target component - loot item
 */
UOrionInteractionTargetComponent_LootItem::UOrionInteractionTargetComponent_LootItem()
{
	ActionDisplayText = FText::FromString("Loot");

	bShouldDestroyActor = false;
}

void UOrionInteractionTargetComponent_LootItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ContainedItems);
}

void UOrionInteractionTargetComponent_LootItem::BeginPlay()
{
	Super::BeginPlay();
}


void UOrionInteractionTargetComponent_LootItem::AddItemToContainer(FOrionPickupDefinition& InPickupDefinition)
{
	ContainedItems.Add(InPickupDefinition);

	EvaluateItemList();
}

void UOrionInteractionTargetComponent_LootItem::TakeAllItems(AActor* Interactor)
{
	int32 IndexToTake = 0;
	for (int32 Index = ContainedItems.Num() - 1; Index >= 0; --Index)
	{
		FOrionPickupDefinition OutPickup;
		IndexToTake = TakeItem(OutPickup, Interactor, IndexToTake) ? IndexToTake : IndexToTake + 1;
	}

	EvaluateItemList();
}

void UOrionInteractionTargetComponent_LootItem::TakeItem(FOrionPickupDefinition& OutDef, bool& bSuccess, AActor* Interactor, int32 ItemIndex)
{
	if(!GetOwner()->HasAuthority())
	{
		Server_TakeItem(OutDef, Interactor, ItemIndex);
	}
	if (!ContainedItems.IsValidIndex(ItemIndex)) return 
	bool bSuccess = false;
	int32 Remainder = 0;
	FItemInstanceData ItemData;

	FOrionPickupDefinition& Pickup = ContainedItems[ItemIndex];
	// Try to add the entire stack into the interactor's inventory
	UOrionInventoryFunctionLibrary::AddInventoryItem(this, Remainder, ItemData, Interactor, Pickup.ItemTag, Pickup.Count);
	if (Remainder <= 0)
	{
		ContainedItems.RemoveAt(ItemIndex);
		bSuccess = true;
	}
	else
	{
		// Partially transferred -> update remaining count
		Pickup.Count = Remainder;
		OutDef = Pickup;
	}
	EvaluateItemList();
	
	return bSuccess;
}

void UOrionInteractionTargetComponent_LootItem::Server_TakeItem_Implementation(FOrionPickupDefinition& OutDef, bool& bSuccess, Actor* Interactor, int32 ItemIndex)
{
	FOrionPickupDefinition OutPickup;
	TakeItem(OutPickup, Interactor, ItemIndex);
}

void UOrionInteractionTargetComponent_LootItem::EvaluateItemList()
{
	// If nothing remains in the lootable, destroy its owner actor
	if (ContainedItems.IsEmpty())
	{
		if (bShouldDestroyActor)
		{
			AActor* OwnerActor = GetOwner();
			if (OwnerActor)
			{
				if (OwnerActor->bNetStartup) UOrionFunctionLibrary::SaveActorData(this, OwnerActor);
				else UOrionFunctionLibrary::RemoveSavedActorData(this, OwnerActor);

				OwnerActor->Destroy();
			}
		}
		else
		{
			Deactivate();
		}
	}
	else
	{
		if (!IsActive()) Activate();
	}
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

void AOrionItemPickup::OnPreSave_Implementation()
{
	ContainedItem = InteractionTargetComp->ContainedItem;
}

void AOrionItemPickup::OnSaved_Implementation()
{
}

void AOrionItemPickup::OnLoaded_Implementation()
{
	InteractionTargetComp->SetItem(ContainedItem);
}

/**
 * Interaction target component - pickup item
 */
UOrionInteractionTargetComponent_PickupItem::UOrionInteractionTargetComponent_PickupItem()
{
	ActionDisplayText = FText::FromString("Pickup");

	bShouldDestroyActor = true;
}


void UOrionInteractionTargetComponent_PickupItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ContainedItem);
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
	EvaluatePickup();
}





void UOrionInteractionTargetComponent_PickupItem::OnLocalCompleted_Implementation(AActor* Interactor, UOrionInteractionTargetComponent* Component)
{
	Super::OnLocalCompleted_Implementation(Interactor, Component);
	int32 Remainder;
	FItemInstanceData ItemData;
	UOrionInventoryFunctionLibrary::AddInventoryItem(this, Remainder, ItemData, Interactor, ContainedItem.ItemTag, ContainedItem.Count);

	if(Remainder <= 0)
	{
		ContainedItem = FOrionPickupDefinition();
		EvaluatePickup();
	}
	else
	{
		ContainedItem.Count = Remainder;
	}

}

void UOrionInteractionTargetComponent_PickupItem::EvaluatePickup()
{
	if (!ContainedItem.ItemTag.IsValid())
	{
		if (bShouldDestroyActor)
		{
			AActor* OwnerActor = GetOwner();
			if (OwnerActor)
			{
				if (OwnerActor->bNetStartup) UOrionFunctionLibrary::SaveActorData(this, OwnerActor);
				else UOrionFunctionLibrary::RemoveSavedActorData(this, OwnerActor);
				OwnerActor->Destroy();
			}
		}
		else
		{
			Deactivate();
		}
	}
	else
	{
		if (!IsActive()) Activate();
	}
}
