// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderFunctionLibrary.h"
#include "OrionRPG.h"
#include "DialogBuilderGraph.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderNode_DialogLine.h"
#include "DialogBuilderNode_OptionSelector.h"
#include "DialogBuilderNode_Root.h"
#include "DialogData.h"
#include "DialogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AssetRegistry/AssetRegistryModule.h"


UDialogComponent* UDialogBuilderFunctionLibrary::GetDialogComponent(const UObject* WorldContextObject)
{
    return GetDialogComponentFromTarget(UGameplayStatics::GetPlayerController(WorldContextObject, 0));
}

UDialogComponent* UDialogBuilderFunctionLibrary::GetDialogComponentFromTarget(AActor* Target)
{
    if (!Target)
    {
        return nullptr;
    }

    if (UDialogComponent* DialogComp = Target->FindComponentByClass<UDialogComponent>())
    {
        return DialogComp;
    }

    //Dialog comp may be on the controllers pawn or pawns controller
    if (APlayerController* OwningController = Cast<APlayerController>(Target))
    {
        if (OwningController->GetPawn())
        {
            return OwningController->GetPawn()->FindComponentByClass<UDialogComponent>();
        }
    }

    if (APawn* OwningPawn = Cast<APawn>(Target))
    {
        if (OwningPawn->GetController())
        {
            return OwningPawn->GetController()->FindComponentByClass<UDialogComponent>();
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("DialogBuilderFunctionLibrary : Current Component Not Found!"));
    return nullptr;
}

UDialogBuilderGraph* UDialogBuilderFunctionLibrary::GetCurrentActiveDialog(const UObject* WorldContextObject)
{
    if (UDialogComponent* DialogComp = GetDialogComponentFromTarget(UGameplayStatics::GetPlayerController(WorldContextObject, 0)))
    {
        return DialogComp->CurrentActiveDialog;
    }

    UE_LOG(LogTemp, Warning, TEXT("DialogBuilderFunctionLibrary : Current Active Dialog Is Null!"));
    return nullptr;
}

UDialogBuilderNode* UDialogBuilderFunctionLibrary::GetCurrentDialogNode(const UObject* WorldContextObject)
{
    UDialogBuilderGraph* CurrentDialogGraph = GetCurrentActiveDialog(WorldContextObject);
    if (CurrentDialogGraph && CurrentDialogGraph->CurrentNode)
    {
        return CurrentDialogGraph->CurrentNode;
    }

    UE_LOG(LogTemp, Warning, TEXT("DialogBuilderFunctionLibrary : Current Dialog Node Is Null!"));
    return nullptr;
}

UDialogBuilderNode_OptionSelector* UDialogBuilderFunctionLibrary::GetLastSelectorNode(const UObject* WorldContextObject)
{
    UDialogBuilderGraph* CurrentDialogGraph = GetCurrentActiveDialog(WorldContextObject);
    if (CurrentDialogGraph && CurrentDialogGraph->LastSelector)
    {
        return CurrentDialogGraph->LastSelector;
    }

    UE_LOG(LogTemp, Warning, TEXT("DialogBuilderFunctionLibrary : Current Dialog Node Is Null!"));
    return nullptr;
}

