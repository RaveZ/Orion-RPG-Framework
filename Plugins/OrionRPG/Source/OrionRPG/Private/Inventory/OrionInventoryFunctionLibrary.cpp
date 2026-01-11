// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionInventoryFunctionLibrary.h"
#include "Inventory/OrionInventoryNode_ItemDefinition.h"
#include "Inventory/OrionInventoryItemInstance.h"
#include "OrionRPG.h"
#include "Inventory/OrionInventoryComponent.h"
#include "Inventory/OrionInventoryGraph.h"
#include "Kismet/GameplayStatics.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UI/OrionUIManagerSubsystem.h"
#include "UI/Foundation/OrionGridBase.h"
#include "UI/Foundation/OrionGridContainerBase.h"
#include "Engine/GameInstance.h"
#include "ContentStreaming.h"
#include "GameFramework/Pawn.h"


UOrionInventoryComponent* UOrionInventoryFunctionLibrary::GetInventoryComponent(const UObject* WorldContextObject)
{
	return GetInventoryComponentFromTarget(UGameplayStatics::GetPlayerController(WorldContextObject, 0));
}

UOrionInventoryComponent* UOrionInventoryFunctionLibrary::GetInventoryComponentFromTarget(AActor* Target)
{
    if (!Target)
    {
        return nullptr;
    }

    if (UOrionInventoryComponent* InventoryComp = Target->FindComponentByClass<UOrionInventoryComponent>())
    {
        return InventoryComp;
    }

    //Inventory comp may be on the controllers pawn or pawns controller
    if (APlayerController* OwningController = Cast<APlayerController>(Target))
    {
        if (OwningController->GetPawn())
        {
            return OwningController->GetPawn()->FindComponentByClass<UOrionInventoryComponent>();
        }
    }

    if (APawn* OwningPawn = Cast<APawn>(Target))
    {
        if (OwningPawn->GetController())
        {
            return OwningPawn->GetController()->FindComponentByClass<UOrionInventoryComponent>();
        }
    }

    return nullptr;
}

UOrionInventoryGraph* UOrionInventoryFunctionLibrary::GetInventoryDB(const UObject* WorldContextObject)
{
    UOrionInventoryComponent* InvComp = GetInventoryComponent(WorldContextObject);
    if (InvComp)
    {
		return InvComp->InventoryDB;
    }
    return nullptr;
}

const UOrionInventoryItemFragment* UOrionInventoryFunctionLibrary::FindItemDefinitionFragment(TSubclassOf<UOrionInventoryNode_ItemDefinition> ItemDef, TSubclassOf<UOrionInventoryItemFragment> FragmentClass)
{
    if ((ItemDef != nullptr) && (FragmentClass != nullptr))
    {
        return GetDefault<UOrionInventoryNode_ItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
    }
    return nullptr;
}

UOrionInventoryItemInstance* UOrionInventoryFunctionLibrary::GetItemInstanceByTag(const UObject* WorldContextObject, AActor* InOptionalInstigator, FGameplayTag ItemTag)
{
    UOrionInventoryComponent* InventoryComp = InOptionalInstigator ? GetInventoryComponentFromTarget(InOptionalInstigator) : GetInventoryComponent(WorldContextObject);

    if (InventoryComp)
    {
        return InventoryComp->GetItemInstanceByTag(ItemTag);
    }

    return nullptr;
}

void UOrionInventoryFunctionLibrary::AssignItemIndex(const UObject* WorldContextObject, UOrionInventoryItemInstance* ItemInstance, int32 TileIndex)
{
    UOrionInventoryComponent* InventoryComp = GetInventoryComponent(WorldContextObject);

    if (InventoryComp)
    {
		InventoryComp->AssignItemIndex(ItemInstance, TileIndex);
    }
}


void UOrionInventoryFunctionLibrary::AddInventoryItem(const UObject* WorldContextObject, int32& StackRemainder, FItemInstanceData& OutItemData, AActor* InOptionalInstigator, FGameplayTag ItemTag, int32 StackCount)
{
    UOrionInventoryComponent* InventoryComp = InOptionalInstigator ? GetInventoryComponentFromTarget(InOptionalInstigator) : GetInventoryComponent(WorldContextObject);

    if (InventoryComp)
    {
        InventoryComp->AddItem(StackRemainder, OutItemData, ItemTag, StackCount);
    }
}

void UOrionInventoryFunctionLibrary::DropInventoryItem(const UObject* WorldContextObject, FItemInstanceData& OutItemData, AActor* InOptionalInstigator, FGameplayTag ItemTag, int32 StackCount)
{
    UOrionInventoryComponent* InventoryComp = InOptionalInstigator ? GetInventoryComponentFromTarget(InOptionalInstigator) : GetInventoryComponent(WorldContextObject);

    if (InventoryComp)
    {
        InventoryComp->DropItem(OutItemData, ItemTag, StackCount);
    }
}

bool UOrionInventoryFunctionLibrary::GetItemData(const UObject* WorldContextObject, FItemInstanceData& OutItemData, AActor* InOptionalInstigator, FGameplayTag ItemTag)
{
    UOrionInventoryComponent* InventoryComp = InOptionalInstigator ? GetInventoryComponentFromTarget(InOptionalInstigator) : GetInventoryComponent(WorldContextObject);
	
    if (InventoryComp)
    {
        return InventoryComp->GetItemData(OutItemData, ItemTag);
    }

    return false;
}

void UOrionInventoryFunctionLibrary::AssignGridItemToSlots(const UObject* WorldContextObject, UOrionGridItem* InGridItem, int32 StartingIndex)
{
    if (InGridItem == nullptr) return;

    UOrionInventoryItemInstance* ItemInstance = InGridItem->ItemInstance;
    TArray<UOrionGridSlot*> ActiveSlots = InGridItem->OwningGridContainer->GridSlots;
    UOrionInventoryComponent* InventoryComp = GetInventoryComponent(WorldContextObject);

    if (InventoryComp && ItemInstance)
    {
        int32 Row = InventoryComp->GetCurrentGridDimension().Y;
        int32 Col = InventoryComp->GetCurrentGridDimension().X;
        FIntPoint ItemSize = ItemInstance->GetItemDef()->GridSize;
        
        for (int32 Y = 0; Y < ItemSize.Y; Y++)
        {
            for (int32 X = 0; X < ItemSize.X; X++)
            {
                int32 SlotIndex = StartingIndex + (Y * Col) + X;
                //UE_LOG(LogTemp, Log, TEXT("Assigning GridItem to SlotIndex: %d"), SlotIndex);
                if (ActiveSlots.IsValidIndex(SlotIndex))
                {
                    ActiveSlots[SlotIndex]->SetGridItem(InGridItem);
                }
            }
        }
    }
}

void UOrionInventoryFunctionLibrary::CreateDragItem(const UObject* WorldContextObject, UOrionInventoryItemInstance* InItemInstance, int32 FromIndex)
{
    UOrionUIManagerSubsystem* UISubsystem = UOrionUILibrary::GetOrionUISubsystem(WorldContextObject);
    if (UISubsystem && UISubsystem->CurrentFocusedGridContainer)
    {
        UISubsystem->CurrentFocusedGridContainer->CreateDragItem(InItemInstance, FromIndex);
    }
}



EOrionInventoryType UOrionInventoryFunctionLibrary::GetInventoryType(const UObject* WorldContextObject)
{
    UOrionInventoryComponent* InventoryComp = GetInventoryComponent(WorldContextObject);

    if (InventoryComp && InventoryComp->InventoryDB)
    {
		return InventoryComp->InventoryDB->InventoryType;
    }
    return EOrionInventoryType();
}

TArray<UOrionInventoryItemInstance*> UOrionInventoryFunctionLibrary::GetAllInventoryItem(const UObject* WorldContextObject)
{
    TArray<UOrionInventoryItemInstance*> OutResult;

    UOrionInventoryComponent* InventoryComp = GetInventoryComponent(WorldContextObject);

    if (InventoryComp)
    {
        OutResult = InventoryComp->GetAllItems();
    }

    return OutResult;
}

bool UOrionInventoryFunctionLibrary::GetEquippedItemFromSlot(const UObject* WorldContextObject, UOrionInventoryItemInstance*& OutItemInstance, UOrionEquipmentInstance*& OutEquipInstance, FGameplayTag InEquipSlot)
{
	UOrionInventoryComponent* InventoryComp = GetInventoryComponent(WorldContextObject);
    if (InventoryComp)
    {
        return InventoryComp->GetEquippedItemFromSlot(OutItemInstance, OutEquipInstance, InEquipSlot);
	}
    return false;
}

UOrionEquipmentInstance* UOrionInventoryFunctionLibrary::EquipItemFromTag(const UObject* WorldContextObject, FGameplayTag ItemTag)
{
    UOrionInventoryComponent* InventoryComp = GetInventoryComponent(WorldContextObject);

    if (InventoryComp)
    {
        return InventoryComp->EquipItemFromTag(ItemTag);
    }
    return nullptr;
}

UOrionEquipmentInstance* UOrionInventoryFunctionLibrary::EquipItem(const UObject* WorldContextObject, FOrionEquipmentDefinition EquipmentDefinition)
{
    UOrionInventoryComponent* InventoryComp = GetInventoryComponent(WorldContextObject);

    if (InventoryComp)
    {
		return InventoryComp->EquipItem(EquipmentDefinition);
    }
    return nullptr;
}

void UOrionInventoryFunctionLibrary::UnequipItem(const UObject* WorldContextObject, UOrionEquipmentInstance* EquipmentInstance)
{
    UOrionInventoryComponent* InventoryComp = GetInventoryComponent(WorldContextObject);

    if (InventoryComp)
    {
		InventoryComp->UnequipItem(EquipmentInstance);
    }
}

void UOrionInventoryFunctionLibrary::LootItemFromActor(const UObject* WorldContextObject, AActor* InActor, AActor* InOptionalInstigator)
{
    UOrionInventoryComponent* InventoryComp = InOptionalInstigator ? GetInventoryComponentFromTarget(InOptionalInstigator) : GetInventoryComponent(WorldContextObject);

    if (InventoryComp)
    {
		InventoryComp->LootItemFromActor(InActor);
    }
}

void UOrionInventoryFunctionLibrary::AddViewOriginToStreaming(const FVector& ViewOrigin, float ScreenWidth, float FOV, float BoostFactor, bool bOverrideLocation, float Duration, AActor* ActorToBoost)
{
    IStreamingManager::Get().AddViewInformation(ViewOrigin,
        ScreenWidth,
        ScreenWidth / FMath::Tan(FMath::DegreesToRadians(FOV * 0.5f)),
        BoostFactor,
        bOverrideLocation,
        Duration,
        ActorToBoost);
}

bool UOrionInventoryFunctionLibrary::GetCurrency(const UObject* WorldContextObject, FGameplayTag InCurrencyTag, FOrionCurrencyData& OutCurrency, AActor* InOptionalInstigator)
{
    UOrionInventoryComponent* InventoryComp = InOptionalInstigator ? GetInventoryComponentFromTarget(InOptionalInstigator) : GetInventoryComponent(WorldContextObject);

    if (InventoryComp)
    {
		return InventoryComp->GetCurrency(InCurrencyTag, OutCurrency);
    }
    return false;
}

bool UOrionInventoryFunctionLibrary::AddCurrency(const UObject* WorldContextObject, FGameplayTag InCurrencyTag, int32 Value, FOrionCurrencyData& OutCurrency, AActor* InOptionalInstigator)
{
    UOrionInventoryComponent* InventoryComp = InOptionalInstigator ? GetInventoryComponentFromTarget(InOptionalInstigator) : GetInventoryComponent(WorldContextObject);

    if (InventoryComp)
    {
        return InventoryComp->AddCurrency(InCurrencyTag, Value, OutCurrency);
    }
    return false;
}

UOrionInventoryNode_ItemDefinition* UOrionInventoryFunctionLibrary::FindItemDefinitionByTag(const UObject* WorldContextObject, FGameplayTag ItemTag)
{
    UOrionInventoryComponent* InventoryComp = GetInventoryComponent(WorldContextObject);

    if (InventoryComp)
    {
        return InventoryComp->FindItemDefinition(ItemTag);
    }

	return nullptr;
}

void UOrionInventoryFunctionLibrary::UnequipItemFromSlot(const UObject* WorldContextObject, FGameplayTag InEquipSlot)
{
    UOrionInventoryComponent* InventoryComp = GetInventoryComponent(WorldContextObject);

    if (InventoryComp)
    {
        InventoryComp->UnequipItemFromSlot(InEquipSlot);
    }
}

