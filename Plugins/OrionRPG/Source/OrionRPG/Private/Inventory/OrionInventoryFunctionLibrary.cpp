// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionInventoryFunctionLibrary.h"
#include "Inventory/OrionInventoryNode_ItemDefinition.h"
#include "Inventory/OrionInventoryItemInstance.h"
#include "OrionRPG.h"
#include "Inventory/OrionInventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AssetRegistry/AssetRegistryModule.h"

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

const UOrionInventoryItemFragment* UOrionInventoryFunctionLibrary::FindItemDefinitionFragment(TSubclassOf<UOrionInventoryNode_ItemDefinition> ItemDef, TSubclassOf<UOrionInventoryItemFragment> FragmentClass)
{
    if ((ItemDef != nullptr) && (FragmentClass != nullptr))
    {
        return GetDefault<UOrionInventoryNode_ItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
    }
    return nullptr;
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