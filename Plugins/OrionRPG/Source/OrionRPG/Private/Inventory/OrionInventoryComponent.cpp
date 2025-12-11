// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionInventoryComponent.h"
#include "Inventory/OrionInventorySetting.h"
#include "Inventory/OrionActorViewer.h"
#include "Inventory/OrionInventoryItemInstance.h"
#include "Inventory/OrionEquipmentInstance.h"
#include "Inventory/OrionInventoryItemFragment.h"
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
{
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
	bReplicateUsingRegisteredSubObjectList = true;
	CurrentWeight = 0;
	bInitializingSaveGame = false;
	CurrentGridDimension = FIntPoint(1, 1);
}


void UOrionInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeDelegate();
	InventoryList.Initialize(this);
	EquipmentList.Initialize(this);
	//Set Dependency
	UWorld* World = GetWorld();
	OwningController = GetOwningController();
	if (InventoryDB)
	{
		InventoryDB->Initialize(this);
		MaxWeight = InventoryDB->DefaultMaxInventoryWeight;
		CurrentGridDimension = InventoryDB->DefaultGridDimension;
	}


	if (World)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = GetOwningPawn();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		//Actor viewer
		TSubclassOf<AOrionActorViewer> ItemViewerClass = GetMutableDefault<UOrionInventorySetting>()->ItemViewerClass;
		ItemViewer = World->SpawnActor<AOrionActorViewer>(ItemViewerClass, GetMutableDefault<UOrionInventorySetting>()->ItemViewerLocation, FRotator::ZeroRotator, SpawnParams);
		
		//Character viewer
		TSubclassOf<AOrionActorViewer> CharacterViewerClass = GetMutableDefault<UOrionInventorySetting>()->CharacterViewerClass;
		CharacterViewer = World->SpawnActor<AOrionActorViewer>(CharacterViewerClass, GetMutableDefault<UOrionInventorySetting>()->CharacterViewerLocation, FRotator::ZeroRotator, SpawnParams);

	}

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

void UOrionInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	OnGridDimensionChanged.RemoveDynamic(this, &ThisClass::GridDimensionChanged);
}

void UOrionInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UOrionInventoryComponent::UninitializeComponent()
{
	TArray<UOrionEquipmentInstance*> AllEquipmentInstances;

	// gathering all instances before removal to avoid side effects affecting the equipment list iterator	
	for (const FOrionEquippedItemEntry& Entry : EquipmentList.Entries)
	{
		AllEquipmentInstances.Add(Entry.Instance);
	}

	for (UOrionEquipmentInstance* EquipInstance : AllEquipmentInstances)
	{
		UnequipItem(EquipInstance);
	}

	Super::UninitializeComponent();
}

void UOrionInventoryComponent::InitializeDelegate()
{
	OnGridDimensionChanged.RemoveDynamic(this, &ThisClass::GridDimensionChanged);

	OnGridDimensionChanged.AddDynamic(this, &ThisClass::GridDimensionChanged);
}



void UOrionInventoryComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InventoryList);
	DOREPLIFETIME(ThisClass, EquipmentList);
}


void UOrionInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UOrionInventoryComponent::AddItem(int32& StackRemainder, FItemInstanceData& OutItemData, FGameplayTag ItemTag, int32 StackCount)
{
	StackRemainder = StackCount;
	if (GetRole() != ROLE_Authority)
	{
		Server_AddItem(ItemTag, StackCount);
		return;
	}

	if (!IsValidItemTag(ItemTag))
	{
		UE_LOG(LogTemp, Error, TEXT("Fail to add item. Invalid item Tag (%s)!"), *ItemTag.ToString());
		return;
	}

	UOrionInventoryItemInstance* ItemInstance = InventoryList.FindFirstItemByTag(ItemTag);
	int32 InitialStackCount = 0;

	if (ItemInstance)
	{
		//Item found, increase stack count instead  
		if (!ItemInstance->HasRoomForItem())
		{
			UE_LOG(LogTemp, Warning, TEXT("Fail to add item: %s. Stack is full (%d/%d)"), *ItemInstance->GetItemDef()->DisplayName.ToString(), ItemInstance->GetTotalStackCount(), ItemInstance->GetItemDef()->MaxStackCount);
			return;
		}
		InitialStackCount = ItemInstance->GetTotalStackCount();
		ItemInstance->SetTotalStackCount(ItemInstance->GetTotalStackCount() + FMath::Abs(StackCount));
		OnItemStackChanged.Broadcast(ItemInstance, ItemInstance->GetTotalStackCount(), ItemInstance->GetItemDef()->MaxStackCount);
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
				InitialStackCount = ItemInstance->GetTotalStackCount();
				ItemInstance->SetTotalStackCount(ItemInstance->GetTotalStackCount() + FMath::Abs(StackCount));
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
		int32 RoomLeft = ItemInstance->GetItemDef()->MaxStackCount - InitialStackCount;

		OutItemData = ItemInstance->GetItemInstanceData();
		StackRemainder = StackCount - FMath::Clamp(StackCount, 0, RoomLeft);
		
		UE_LOG(LogTemp, Log, TEXT("Item Added: %s | Tag: %s, StackCount: %d/%d\n"), *ItemInstance->GetItemDef()->DisplayName.ToString(), *ItemInstance->GetItemDef()->Tag.ToString(), ItemInstance->GetTotalStackCount(), ItemInstance->GetItemDef()->MaxStackCount);
	}
}



void UOrionInventoryComponent::Server_AddItem_Implementation(FGameplayTag ItemTag, int32 StackCount)
{
	int32 Remainder;
	FItemInstanceData ItemData;
	AddItem(Remainder, ItemData, ItemTag, StackCount);
}

void UOrionInventoryComponent::DropItem(FItemInstanceData& OutItemData, FGameplayTag ItemTag, int32 StackCount)
{
	if (GetRole() != ROLE_Authority)
	{
		Server_DropItem(ItemTag, StackCount);
		return;
	}

	if (!IsValidItemTag(ItemTag))
	{
		UE_LOG(LogTemp, Error, TEXT("Fail to drop item. Invalid item Tag (%s)!"), *ItemTag.ToString());
		return;
	}

	UOrionInventoryItemInstance* ItemInstance = InventoryList.FindFirstItemByTag(ItemTag);

	if (ItemInstance)
	{
		Server_DropItemInstance(ItemInstance, StackCount);
		OutItemData = ItemInstance->GetItemInstanceData();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Fail to drop item. Item with Tag (%s) not found in inventory!"), *ItemTag.ToString());
	}
}


void UOrionInventoryComponent::Server_DropItem_Implementation(FGameplayTag ItemTag, int32 StackCount)
{
	FItemInstanceData ItemData;
	DropItem(ItemData, ItemTag, StackCount);
}

UOrionInventoryNode_ItemDefinition* UOrionInventoryComponent::FindItemDefinition(FGameplayTag ItemTag)
{
	if(!IsValidItemTag(ItemTag))
	{
		return nullptr;
	}
	if(InventoryDB)
	{
		return InventoryDB->ItemDefinitions.FindRef(ItemTag);
	}
	return nullptr;
}



void UOrionInventoryComponent::Server_DropItemInstance_Implementation(UOrionInventoryItemInstance* ItemInstance, int32 StackCount)
{
	if (!ItemInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fail to drop item. ItemInstance is null!"));
		return;
	}

	const int32 NewStackCount = ItemInstance->GetTotalStackCount() - FMath::Abs(StackCount);
	ItemInstance->SetTotalStackCount(NewStackCount);
	
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

void UOrionInventoryComponent::ConsumeItem(FItemInstanceData& OutItemData, FGameplayTag ItemTag, int32 StackCount)
{
	if (GetRole() != ROLE_Authority)
	{
		Server_DropItem(ItemTag, StackCount);
		return;
	}

	if (!IsValidItemTag(ItemTag))
	{
		UE_LOG(LogTemp, Error, TEXT("Fail to consume item. Invalid item Tag (%s)!"), *ItemTag.ToString());
		return;
	}

	UOrionInventoryItemInstance* ItemInstance = InventoryList.FindFirstItemByTag(ItemTag);

	if (ItemInstance)
	{
		Server_ConsumeItemInstance(ItemInstance, StackCount);
		OutItemData = ItemInstance->GetItemInstanceData();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Fail to consume item. Item with Tag (%s) not found in inventory!"), *ItemTag.ToString());
	}
}

void UOrionInventoryComponent::Server_ConsumeItemInstance_Implementation(UOrionInventoryItemInstance* ItemInstance, int32 StackCount)
{
	if (!ItemInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fail to consume item. ItemInstance is null!"));
		return;
	}

	const int32 NewStackCount = ItemInstance->GetTotalStackCount() - FMath::Abs(StackCount);
	ItemInstance->SetTotalStackCount(NewStackCount);

	if (NewStackCount <= 0)
	{
		InventoryList.RemoveEntry(ItemInstance);
		if (IsUsingRegisteredSubObjectList())
		{
			RemoveReplicatedSubObject(ItemInstance);
		}
		OnItemConsumed.Broadcast(ItemInstance);
	}
	else
	{
		OnItemStackChanged.Broadcast(ItemInstance, NewStackCount, ItemInstance->GetItemDef()->MaxStackCount);
	}
	UE_LOG(LogTemp, Log, TEXT("Consume Item: %s | Tag: %s, StackCount: %d/%d\n"), *ItemInstance->GetItemDef()->DisplayName.ToString(), *ItemInstance->GetItemDef()->Tag.ToString(), ItemInstance->GetTotalStackCount(), ItemInstance->GetItemDef()->MaxStackCount);



}

void UOrionInventoryComponent::Server_ConsumeItem_Implementation(FGameplayTag ItemTag, int32 StackCount)
{
	FItemInstanceData ItemData;
	ConsumeItem(ItemData, ItemTag, StackCount);

}

void UOrionInventoryComponent::SetInventoryMaxWeight(float InNewMaxWeight)
{
	if (MaxWeight == InNewMaxWeight)
	{
		return;
	}

	MaxWeight = FMath::Max(0, InNewMaxWeight);
	OnMaxWeightChanged.Broadcast(MaxWeight);
}

void UOrionInventoryComponent::SetInventoryGridDimension(FIntPoint InNewDimension)
{
	FIntPoint OldDimension = CurrentGridDimension;
	if (CurrentGridDimension == InNewDimension ||
		InNewDimension.X < CurrentGridDimension.X ||
		InNewDimension.Y < CurrentGridDimension.Y)
	{
		UE_LOG(LogTemp, Log, TEXT("Set inventory grid dimension failed, new dimension may be the same or smaller. \n Old: (%d, %d), New: (%d, %d)"),
			OldDimension.X, OldDimension.Y, InNewDimension.X, InNewDimension.Y);
		return;
	}
	CurrentGridDimension = InNewDimension;
	InventoryList.UpdateGrid(OldDimension, CurrentGridDimension);
	OnGridDimensionChanged.Broadcast(OldDimension, CurrentGridDimension);
}

TArray<UOrionInventoryItemInstance*> UOrionInventoryComponent::GetAllItems() const
{
	return InventoryList.GetAllItems();
}


bool UOrionInventoryComponent::IsValidSlot(UOrionInventoryNode_ItemDefinition* ItemDef, int32 SlotIndex)
{
	return InventoryList.IsValidSlot(ItemDef, SlotIndex);
}

void UOrionInventoryComponent::SetEntries(TArray<UOrionInventoryItemInstance*> InItemList)
{
	InventoryList.SetEntries(InItemList);	
	

}

void UOrionInventoryComponent::AssignItemIndex(UOrionInventoryItemInstance* ItemInstance, int32 TileIndex)
{
	InventoryList.AssignItemIndex(ItemInstance, TileIndex);
}

UOrionInventoryItemInstance* UOrionInventoryComponent::GetItemInstanceByTag(FGameplayTag ItemTag)
{
	return InventoryList.FindFirstItemByTag(ItemTag);
}

bool UOrionInventoryComponent::GetItemData(FItemInstanceData& OutItemData, FGameplayTag ItemTag)
{
	UOrionInventoryItemInstance* ItemInstance = GetItemInstanceByTag(ItemTag);

	if (ItemInstance)
	{
		OutItemData = ItemInstance->GetItemInstanceData();
		return true;
	}
	return false;
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

void UOrionInventoryComponent::InspectItem(FGameplayTag ItemTag, TSoftClassPtr<AActor> ActorToInspect)
{
	UOrionInventoryItemInstance* FoundItem = InventoryList.FindFirstItemByTag(ItemTag);
	
	if (FoundItem)
	{
		InspectdItem = FoundItem;
		InspectItemInstance(FoundItem, ActorToInspect);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Fail to Inspect item. Item not found!"));
	}
}

void UOrionInventoryComponent::InspectItemInstance(UOrionInventoryItemInstance* ItemInstance, TSoftClassPtr<AActor> ActorToInspect)
{
	if (!ActorToInspect && !ItemInstance)
	{
		return;
	}

	TSoftClassPtr<AActor> InActorClass = ActorToInspect ? ActorToInspect : ItemInstance->GetItemDef()->GetPickupActorClass();

	OnInspectItem.Broadcast(ItemInstance, InActorClass);

}

UOrionEquipmentInstance* UOrionInventoryComponent::EquipItem(FOrionEquipmentDefinition EquipmentDefinition)
{
	if (EquipmentDefinition.EquipSlot == FGameplayTag::EmptyTag)
	{
		UE_LOG(LogTemp, Error, TEXT("Fail to add item. Invalid Equipment Slot Tag"));
		return nullptr;
	}
	
	UOrionEquipmentInstance* Result = nullptr;
	if (GetRole() != ROLE_Authority)
	{
		Server_EquipItem(EquipmentDefinition);
		return nullptr;
	}

	
	Result = EquipmentList.SetEntry(EquipmentDefinition);
	if (Result != nullptr)
	{
		Result->OnEquipped();

		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
		{
			AddReplicatedSubObject(Result);
		}
	}
	
	return Result;
}

void UOrionInventoryComponent::Server_EquipItem_Implementation(FOrionEquipmentDefinition EquipmentDefinition)
{
	EquipItem(EquipmentDefinition);
}



void UOrionInventoryComponent::UnequipItemFromSlot(FGameplayTag InEquipSlot)
{
	if (InEquipSlot == FGameplayTag::EmptyTag)
	{
		UE_LOG(LogTemp, Error, TEXT("Fail to add item. Invalid Equipment Slot Tag"));
	}
	UOrionEquipmentInstance* EquipmentInstance = EquipmentList.FindFirstEquipmentBySlot(InEquipSlot);

	if (EquipmentInstance)
	{
		UnequipItem(EquipmentInstance);
	}
}

void UOrionInventoryComponent::UnequipItem(UOrionEquipmentInstance* EquipmentInstance)
{
	if (GetRole() != ROLE_Authority)
	{
		Server_UnequipItem(EquipmentInstance);
		return;
	}
	if (EquipmentInstance != nullptr)
	{
		if (IsUsingRegisteredSubObjectList())
		{
			RemoveReplicatedSubObject(EquipmentInstance);
		}

		EquipmentInstance->OnUnequipped();
		EquipmentList.RemoveEntry(EquipmentInstance);
	}
}

void UOrionInventoryComponent::Server_UnequipItem_Implementation(UOrionEquipmentInstance* EquipmentInstance)
{
	UnequipItem(EquipmentInstance);
}

FOrionInventorySaveData UOrionInventoryComponent::GetSaveGameData()
{
	FOrionInventorySaveData InventorySaveData;
	TArray<UOrionInventoryItemInstance*> AllItems = GetAllItems();
	InventorySaveData.SavedInventoryItems.SetNum(AllItems.Num());
    for (int32 i = 0; i < AllItems.Num(); ++i)
    {
		if (AllItems.IsValidIndex(i))
		{
			auto ItemInstance = AllItems[i];
			if (ItemInstance && ItemInstance->GetItemDef())
			{
				FOrionInventoryItemSaveData ItemSaveData;
				ItemSaveData.ItemTag = ItemInstance->GetItemDef()->Tag;
				ItemSaveData.TotalStackCount = ItemInstance->GetTotalStackCount();

				InventorySaveData.SavedInventoryItems[i] = ItemSaveData;
			}
		}
		
    }
	
	InventorySaveData.MaxWeight = MaxWeight;
	InventorySaveData.GridDimension = CurrentGridDimension;

	return InventorySaveData;
}

void UOrionInventoryComponent::InitializeFromSaveGame(const FOrionInventorySaveData& InventorySaveData)
{
	if (GetRole() != ROLE_Authority)
	{
		Server_InitializeFromSaveGame(InventorySaveData);
		return;
	}
	//Reset Inventory List
	bInitializingSaveGame = true;
	InventoryList.Reset();
	InventoryList.InitializeFromSaveGame(InventorySaveData);

	MaxWeight = InventorySaveData.MaxWeight;
	CurrentGridDimension = InventorySaveData.GridDimension;
	
	bInitializingSaveGame = false;

}

void UOrionInventoryComponent::Server_InitializeFromSaveGame_Implementation(const FOrionInventorySaveData& InventorySaveData)
{
	InitializeFromSaveGame(InventorySaveData);
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

	for (FOrionEquippedItemEntry& Entry : EquipmentList.Entries)
	{
		UOrionEquipmentInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
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
	// Register existing OrionEquipmentInstances
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FOrionEquippedItemEntry& Entry : EquipmentList.Entries)
		{
			UOrionEquipmentInstance* Instance = Entry.Instance;

			if (IsValid(Instance))
			{
				AddReplicatedSubObject(Instance);
			}
		}
	}
}

float UOrionInventoryComponent::GetCurrentWeight()
{
	TArray<UOrionInventoryItemInstance*> AllItems;
	for (auto& Item : GetAllItems())
	{
		AllItems.AddUnique(Item);
	}

	float OutWeight = 0;

	for (int i = 0; i < AllItems.Num(); i++)
	{
		if (AllItems.IsValidIndex(i))
		{
			UOrionInventoryItemInstance* ItemInstance = AllItems[i];
			if (ItemInstance)
			{
				OutWeight += (ItemInstance->GetTotalStackCount() * ItemInstance->GetItemInstanceData().Weight);
			}
		}
	}
	return OutWeight;
}

float UOrionInventoryComponent::GetMaxWeight()
{
	return MaxWeight;
}

FIntPoint UOrionInventoryComponent::GetCurrentGridDimension()
{
	FIntPoint OutDimension;
    OutDimension.X = CurrentGridDimension.X;
    int32 ItemMaxColumn = CurrentGridDimension.X > 0 ? FMath::CeilToInt(static_cast<float>(GetAllItems().Num()) / static_cast<float>(CurrentGridDimension.X)) : CurrentGridDimension.Y;
	OutDimension.Y = InventoryDB ? InventoryDB->InventoryType == EOrionInventoryType::E_GridBased ? CurrentGridDimension.Y : FMath::Max(ItemMaxColumn, CurrentGridDimension.Y) : ItemMaxColumn;
	return OutDimension;
}

void UOrionInventoryComponent::GridDimensionChanged(FIntPoint OldDimension, FIntPoint NewDimension)
{
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

	int32 Remainder;
	FItemInstanceData ItemData;
	AddItem(Remainder, ItemData, FGameplayTag::RequestGameplayTag(FName(*ItemTag[0])));
}

void UOrionInventoryComponent::execDropItem(const TArray<FString>& ItemTag)
{
	if (ItemTag.IsEmpty()) return;

	FItemInstanceData ItemData;
	DropItem(ItemData, FGameplayTag::RequestGameplayTag(FName(*ItemTag[0])));
}


#undef LOCTEXT_NAMESPACE