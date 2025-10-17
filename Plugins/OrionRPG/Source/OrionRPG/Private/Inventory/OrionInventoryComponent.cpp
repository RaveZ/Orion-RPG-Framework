// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionInventoryComponent.h"
#include "Inventory/OrionInventoryItemInstance.h"
#include "Inventory/OrionInventoryNode_ItemDefinition.h"
#include "Decorator/OrionDecorator.h"
#include "Event/OrionEvent.h"
#include "Inventory/InventoryData.h"
#include "UObject/UObjectIterator.h"
#include "Inventory/OrionInventoryGraph.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Controller.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "NativeGameplayTags.h"
#include "Engine/ActorChannel.h"
#include "Engine/World.h"




#define LOCTEXT_NAMESPACE "OrionInventoryComponent"

UOrionInventoryComponent::UOrionInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventoryList(this)
{
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
	DefaultMaxInventoryWeight = 100.0f;
	CurrentWeight = 0;
}

void UOrionInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	//Set Dependency
	OwningController = GetOwningController();
	MaxWeight = DefaultMaxInventoryWeight;
}


void UOrionInventoryComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InventoryList);
}


void UOrionInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UOrionInventoryComponent::AddItem(FGameplayTag ItemTag, int32 Count, FInventoryItem& OutItem)
{
	if (!ItemTag.MatchesTag(TAG_Item))
	{
		UE_LOG(LogTemp, Error, TEXT("Fail to add item. Invalid item Tag (%s)!"), *ItemTag.ToString());
		return;
	}

	UOrionInventoryItemInstance* FoundItem = InventoryList.FindFirstItemByTag(ItemTag);

	if (FoundItem)
	{
		//Item found, increase stack count instead  
		//TODO: broadcast event on stack count changed
		FoundItem->SetTotalStackCount(FoundItem->GetTotalStackCount() + Count);
		//OnItemStackChanged.Broadcast(FoundItem, FoundItem->GetTotalStackCount(), FoundItem->GetItemDef()->ItemDefinition);
	}
	else
	{
		//Item not found, create new entry
		UOrionInventoryNode_ItemDefinition* ItemDef = FindItemDefinition(ItemTag);

		if (ItemDef)
		{
			UOrionInventoryItemInstance* Result = nullptr;
			Result = InventoryList.AddEntry(ItemDef);

			if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && Result)
			{
				AddReplicatedSubObject(Result);
				if (GetOwner()->GetNetMode() == NM_ListenServer || GetOwner()->GetNetMode() == NM_Standalone)
				{
					OnItemAdded.Broadcast(Result);
				}
			}
		}
	}
	
}

UOrionInventoryNode_ItemDefinition* UOrionInventoryComponent::FindItemDefinition(FGameplayTag ItemTag)
{
	if (!ItemTag.MatchesTag(TAG_Item))
	{
		UE_LOG(LogTemp, Error, TEXT("Fail to add item. Invalid item Tag (%s)!"), *ItemTag.ToString());
		return nullptr;
	}

	if(InventoryDB)
	{
		return InventoryDB->ItemDefinitions.FindRef(ItemTag);
	}
	return nullptr;
}


void UOrionInventoryComponent::DropItem(UOrionInventoryItemInstance* ItemInstance, int32 StackCount)
{

}

void UOrionInventoryComponent::SetInventoryMaxWeight(float InNewMaxWeight)
{
	if(MaxWeight == InNewMaxWeight)
	{
		return;
	}
	MaxWeight = InNewMaxWeight;
	OnMaxWeightChanged.Broadcast(MaxWeight);
}

APawn* UOrionInventoryComponent::GetOwningPawn() const
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

APlayerController* UOrionInventoryComponent::GetOwningController() const
{
	//We cache this on beginplay as to not re-find it every time 
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


bool UOrionInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (FOrionInventoryEntry& Entry : InventoryList.Entries)
	{
		UOrionInventoryItemInstance* Instance = Entry.ItemInstance;

		if (Instance && IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

void UOrionInventoryComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// Register existing UOrionInventoryItemInstance
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FOrionInventoryEntry& Entry : InventoryList.Entries)
		{
			UOrionInventoryItemInstance* Instance = Entry.ItemInstance;

			if (IsValid(Instance))
			{
				AddReplicatedSubObject(Instance);
			}
		}
	}
}


#undef LOCTEXT_NAMESPACE