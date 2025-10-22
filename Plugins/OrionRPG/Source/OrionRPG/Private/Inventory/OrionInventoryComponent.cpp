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

	//console command
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("orionLogInventory"),
		TEXT("Dump all inventory item held by player"),
		FConsoleCommandDelegate::CreateUObject(this, &ThisClass::execLogInventory),
		ECVF_Default);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("orionAddItem"),
		TEXT("[ItemTag] Add item to inventory"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &ThisClass::execAddItem),
		ECVF_Default);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("orionDropItem"),
		TEXT("[ItemTag] Drop inventory item"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &ThisClass::execDropItem),
		ECVF_Default);
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

void UOrionInventoryComponent::AddItem(FGameplayTag ItemTag, int32 StackCount)
{
	if (GetRole() != ROLE_Authority)
	{
		Server_AddItem(ItemTag, StackCount);
		return;
	}

	if (!ItemTag.MatchesTag(TAG_Item))
	{
		UE_LOG(LogTemp, Error, TEXT("Fail to add item. Invalid item Tag (%s)!"), *ItemTag.ToString());
		return;
	}

	UOrionInventoryItemInstance* ItemInstance = InventoryList.FindFirstItemByTag(ItemTag);

	if (ItemInstance)
	{
		//Item found, increase stack count instead  
		//TODO: broadcast event on stack count changed
		if (!ItemInstance->HasRoomForItem())
		{
			UE_LOG(LogTemp, Warning, TEXT("Fail to add item: %s. Stack is full (%d/%d)"), *ItemInstance->GetItemDef()->DisplayName.ToString(), ItemInstance->GetTotalStackCount(), ItemInstance->GetItemDef()->MaxStackCount);
			return;
		}
		OnItemStackChanged.Broadcast(ItemInstance, ItemInstance->GetTotalStackCount(), ItemInstance->GetItemDef()->MaxStackCount);
		ItemInstance->SetTotalStackCount(FMath::Clamp(ItemInstance->GetTotalStackCount() + StackCount, 0, ItemInstance->GetItemDef()->MaxStackCount));
	}
	else
	{
		//Item not found, create new entry
		UOrionInventoryNode_ItemDefinition* ItemDef = FindItemDefinition(ItemTag);

		if (ItemDef)
		{
			ItemInstance = InventoryList.AddEntry(const_cast<UOrionInventoryNode_ItemDefinition*>(ItemDef));
			if (ItemInstance && this->IsUsingRegisteredSubObjectList() && this->IsReadyForReplication())
			{
				this->AddReplicatedSubObject(ItemInstance);
				ItemInstance->SetTotalStackCount(FMath::Clamp(ItemInstance->GetTotalStackCount() + StackCount, 0, ItemInstance->GetItemDef()->MaxStackCount));
				AActor* OwnerActor = this->GetOwner();
				if (OwnerActor && (OwnerActor->GetNetMode() == NM_ListenServer || OwnerActor->GetNetMode() == NM_Standalone))
				{
					OnItemAdded.Broadcast(ItemInstance);
				}
			}
		}
	}

	if (ItemInstance)
	{
		UE_LOG(LogTemp, Log, TEXT("Item Added: %s | Tag: %s, StackCount: %d/%d\n"), *ItemInstance->GetItemDef()->DisplayName.ToString(), *ItemInstance->GetItemDef()->Tag.ToString(), ItemInstance->GetTotalStackCount(), ItemInstance->GetItemDef()->MaxStackCount);
	}
}



void UOrionInventoryComponent::Server_AddItem_Implementation(FGameplayTag ItemTag, int32 StackCount)
{
	AddItem(ItemTag, StackCount);
}

void UOrionInventoryComponent::DropItem(FGameplayTag ItemTag, int32 StackCount)
{
	if (GetRole() != ROLE_Authority)
	{
		Server_DropItem(ItemTag, StackCount);
		return;
	}

	if (!ItemTag.MatchesTag(TAG_Item))
	{
		UE_LOG(LogTemp, Error, TEXT("Fail to add item. Invalid item Tag (%s)!"), *ItemTag.ToString());
		return;
	}

	UOrionInventoryItemInstance* ItemInstance = InventoryList.FindFirstItemByTag(ItemTag);

	if (ItemInstance)
	{
		DropItemInstance(ItemInstance, StackCount);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Fail to drop item. Item with Tag (%s) not found in inventory!"), *ItemTag.ToString());
	}
}


void UOrionInventoryComponent::Server_DropItem_Implementation(FGameplayTag ItemTag, int32 StackCount)
{
	DropItem(ItemTag, StackCount);
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



void UOrionInventoryComponent::DropItemInstance(UOrionInventoryItemInstance* ItemInstance, int32 StackCount)
{
	if (!ItemInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fail to drop item. ItemInstance is null!"));
		return;
	}

	const int32 NewStackCount = ItemInstance->GetTotalStackCount() - FMath::Abs(StackCount);
	ItemInstance->SetTotalStackCount(FMath::Clamp(NewStackCount, 0, ItemInstance->GetItemDef()->MaxStackCount));
	
	if(NewStackCount <= 0)
	{
		InventoryList.RemoveEntry(ItemInstance);
		if (IsUsingRegisteredSubObjectList())
		{
			RemoveReplicatedSubObject(ItemInstance);
		}
		OnItemRemoved.Broadcast(ItemInstance);
	}
	else
	{
		OnItemStackChanged.Broadcast(ItemInstance, NewStackCount, ItemInstance->GetItemDef()->MaxStackCount);
	}
	UE_LOG(LogTemp, Log, TEXT("Drop Item: %s | Tag: %s, StackCount: %d/%d\n"), *ItemInstance->GetItemDef()->DisplayName.ToString(), *ItemInstance->GetItemDef()->Tag.ToString(), ItemInstance->GetTotalStackCount(), ItemInstance->GetItemDef()->MaxStackCount);



	//TODO: Spawn dropped item in the world

}

TArray<UOrionInventoryItemInstance*> UOrionInventoryComponent::GetAllItems() const
{
	return InventoryList.GetAllItems();
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


// ================================================================================================ //
// CONSOLE COMMANDS
// ================================================================================================ //

void UOrionInventoryComponent::execLogInventory()
{
#if !UE_BUILD_SHIPPING

	static FString Message;
	Message.Reset();

	Message += TEXT("\n==================== BEGIN Inventory ====================\n");

	TArray<UOrionInventoryItemInstance* > ItemList = InventoryList.GetAllItems();
	for (int32 i = 0; i < ItemList.Num(); ++i)
	{
		if (ItemList.IsValidIndex(i))
		{
			UOrionInventoryItemInstance* Item = ItemList[i];
			if (Item)
			{
				Message += FString::Printf(TEXT("Name: %s | Tag: %s, StackCount: %d/%d\n"), *Item->GetItemDef()->DisplayName.ToString(), *Item->GetItemDef()->Tag.ToString(), Item->GetTotalStackCount(), Item->GetItemDef()->MaxStackCount);
			}
		}
	}


	Message += TEXT("==================== END Inventory ====================\n");

	UE_LOG(LogTemp, Log, TEXT(" %s"), *Message);

#endif // UE_BUILD_SHIPPING
}

void UOrionInventoryComponent::execAddItem(const TArray<FString>& ItemTag)
{
	if (ItemTag.IsEmpty()) return;

	AddItem(FGameplayTag::RequestGameplayTag(FName(*ItemTag[0])));
}

void UOrionInventoryComponent::execDropItem(const TArray<FString>& ItemTag)
{
	if (ItemTag.IsEmpty()) return;

	DropItem(FGameplayTag::RequestGameplayTag(FName(*ItemTag[0])));
}


#undef LOCTEXT_NAMESPACE