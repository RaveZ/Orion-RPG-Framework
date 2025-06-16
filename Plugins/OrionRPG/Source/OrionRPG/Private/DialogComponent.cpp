// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogComponent.h"
#include "Decorator/OrionDecorator.h"
#include "Event/OrionEvent.h"
#include "UObject/UObjectIterator.h"
#include "DialogSaveObject.h"
#include "DialogBuilderGraph.h"
#include "DialogBuilderNode_OptionSelector.h"
#include "DialogBuilderNode_Root.h"
#include "DialogBuilderNode_DialogLine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Controller.h"
#include "DialogData.h"
#include "AssetRegistry/AssetRegistryModule.h"




#define LOCTEXT_NAMESPACE "DialogComponent"

UDialogComponent::UDialogComponent()
{
	SetIsReplicatedByDefault(true);
	SetComponentTickEnabled(true);
	PrimaryComponentTick.bCanEverTick = true;
}

void UDialogComponent::BeginPlay()
{
	Super::BeginPlay();
	DialogMap.Empty();
	//Set Dependency
	OwningController = GetOwningController();
	
	OnDialogUpdated.AddDynamic(this, &UDialogComponent::DialogLineUpdated);
	OnEnterOptionSelector.AddDynamic(this, &UDialogComponent::EnterOptionSelector);
	OnPLayerOptionSelected.AddDynamic(this, &UDialogComponent::PlayerOptionSelected);
	OnBeginDialog.AddDynamic(this, &UDialogComponent::DialogBegin);
	OnEndDialog.AddDynamic(this, &UDialogComponent::DialogEnd);
}


void UDialogComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

bool UDialogComponent::BeginDialog(UDialogBuilderGraph* DialogAsset)
{
	if (DialogAsset == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Dialog Asset is null"));
		return false;
	}
	if (CurrentActiveDialog)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't begin dialog, make sure current active dialog has finished before starting another dialog"));
		return false;
	}

	bool bSuccess = false;

	UDialogBuilderGraph* NewDialogGraph = MakeDialogGraphInstance(DialogAsset);
	if (NewDialogGraph)
	{
		NewDialogGraph->Initialize(this);
		CurrentActiveDialog = NewDialogGraph;
		
		if (NewDialogGraph->RootNodes.IsValidIndex(0))
		{
			NewDialogGraph->BeginNode(NewDialogGraph->RootNodes[0]);
			OnBeginDialog.Broadcast(NewDialogGraph);
		}
		bSuccess = true;
	}
	
	return bSuccess;
}

class UDialogBuilderGraph* UDialogComponent::MakeDialogGraphInstance(UDialogBuilderGraph* DialogGraphTemplate)
{
	if (IsValid(DialogGraphTemplate))
	{
		//Duplicate the dialog graph
		UDialogBuilderGraph* NewDialogGraph = Cast<UDialogBuilderGraph>(StaticDuplicateObject(DialogGraphTemplate, this, NAME_None, RF_Transactional));
		return NewDialogGraph;
	}

	return nullptr;
}

APawn* UDialogComponent::GetOwningPawn() const
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

APlayerController* UDialogComponent::GetOwningController() const
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

void UDialogComponent::Save_Implementation(const FString& SaveFileName, int SlotIndex)
{
}

void UDialogComponent::Load_Implementation(const FString& SaveFileName, int SlotIndex) 
{
	
}

bool UDialogComponent::DeleteSave(const FString& SaveName, const int32 Slot)
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveName, 0))
	{
		return false;
	}

	return UGameplayStatics::DeleteGameInSlot(SaveName, Slot);
}


void UDialogComponent::DialogLineUpdated(UDialogBuilderNode* DialogNode)
{
}

void UDialogComponent::EnterOptionSelector(const TArray<class UDialogBuilderNode_PlayerOption*>& PlayerOptions)
{
}

void UDialogComponent::PlayerOptionSelected(int32 SelectionIndex)
{
    UE_LOG(LogTemp, Log, TEXT("Dialog Option Selected: %d"), SelectionIndex);
}

void UDialogComponent::DialogBegin(UDialogBuilderGraph* Dialog)
{
	if (Dialog)
	{
		UE_LOG(LogTemp, Log, TEXT("Begin Dialog: %s"), *Dialog->ID.ToString());
	}
}

void UDialogComponent::DialogEnd(UDialogBuilderGraph* Dialog)
{
	CurrentActiveDialog = nullptr;
	if (Dialog)
	{
		UE_LOG(LogTemp, Log, TEXT("End Dialog: %s"), *Dialog->ID.ToString());
	}
}

#undef LOCTEXT_NAMESPACE