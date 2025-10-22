// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionInventoryFunctionLibrary.h"
#include "Inventory/OrionInventoryNode_ItemDefinition.h"
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


void UOrionInventoryFunctionLibrary::AddInventoryItem(const UObject* WorldContextObject, AActor* InOptionalInteractor, FGameplayTag ItemTag, int32 StackCount)
{
    UOrionInventoryComponent* InventoryComp = InOptionalInteractor ? GetInventoryComponentFromTarget(InOptionalInteractor) : GetInventoryComponent(WorldContextObject);

    if (InventoryComp)
    {
        InventoryComp->AddItem(ItemTag, StackCount);
    }
}