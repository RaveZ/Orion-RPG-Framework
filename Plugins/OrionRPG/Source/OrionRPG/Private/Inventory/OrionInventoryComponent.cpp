// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionInventoryComponent.h"
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




#define LOCTEXT_NAMESPACE "OrionInventoryComponent"

UOrionInventoryComponent::UOrionInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
}

void UOrionInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	OrionInventoryMap.Empty();
	//Set Dependency
	OwningController = GetOwningController();
}

void UOrionInventoryComponent::AddItem(FGameplayTag Tag, int32 Count, FInventoryItem& OutItem)
{
	if (!Tag.MatchesTag(TAG_Item))
	{
		UE_LOG(LogTemp, Error, TEXT("Fail to add item. Invalid item Tag (%s)!"), *Tag.ToString());
		return;
	}



	//FInventoryItem& NewItem = OrionInventoryMap.FindOrAdd(Tag);
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



#undef LOCTEXT_NAMESPACE