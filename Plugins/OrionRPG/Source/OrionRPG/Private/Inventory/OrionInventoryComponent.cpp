// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Inventory/OrionInventoryComponent.h"
#include "Decorator/OrionDecorator.h"
#include "Event/OrionEvent.h"
#include "UObject/UObjectIterator.h"
#include "Inventory/OrionInventoryGraph.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Controller.h"
#include "AssetRegistry/AssetRegistryModule.h"




#define LOCTEXT_NAMESPACE "OrionInventoryComponent"

UOrionInventoryComponent::UOrionInventoryComponent()
{
	SetIsReplicatedByDefault(true);
	SetComponentTickEnabled(true);
	PrimaryComponentTick.bCanEverTick = true;
}

void UOrionInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	OrionInventoryMap.Empty();
	//Set Dependency
	OwningController = GetOwningController();
}


void UOrionInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

bool UOrionInventoryComponent::BeginOrionInventory(UOrionInventoryGraph* OrionInventoryAsset)
{
	if (OrionInventoryAsset == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("OrionInventory Asset is null"));
		return false;
	}

	bool bSuccess = false;

	return bSuccess;
}

class UOrionInventoryGraph* UOrionInventoryComponent::MakeOrionInventoryGraphInstance(UOrionInventoryGraph* OrionInventoryGraphTemplate)
{

	return nullptr;
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

void UOrionInventoryComponent::Save_Implementation(const FString& SaveFileName, int SlotIndex)
{
}

void UOrionInventoryComponent::Load_Implementation(const FString& SaveFileName, int SlotIndex) 
{
	
}

bool UOrionInventoryComponent::DeleteSave(const FString& SaveName, const int32 Slot)
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveName, 0))
	{
		return false;
	}

	return UGameplayStatics::DeleteGameInSlot(SaveName, Slot);
}


#undef LOCTEXT_NAMESPACE