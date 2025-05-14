// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderFunctionLibrary.h"
#include "Dialog_System_Runtime.h"
#include "DialogBuilderGraph.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderNode_Root.h"
#include "Dialog.h"
#include "DialogData.h"
#include "DialogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AssetRegistry/AssetRegistryModule.h"

TArray<FName> UDialogBuilderFunctionLibrary::GetAllDialogID()
{
    TArray<FName> DialogIDs;

    // Load all UDialogBuilderGraph assets
    TArray<FAssetData> AssetDataArray;
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UDialogBuilderGraph::StaticClass()), AssetDataArray);

    for (const FAssetData& AssetData : AssetDataArray)
    {
        UDialogBuilderGraph* DialogGraph = Cast<UDialogBuilderGraph>(AssetData.GetAsset());
        if (DialogGraph)
        {
            for (UDialog* Dialog : DialogGraph->DialogList)
            {
                DialogIDs.Add(Dialog->ID);
            }
        }
    }

    return DialogIDs;
}

TArray<FName> UDialogBuilderFunctionLibrary::GetNodeIDFromCachedDialog()
{
	TArray<FName> NodeIDs;
    // Load all UDialogBuilderGraph assets
    TArray<FAssetData> AssetDataArray;
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UDialogBuilderGraph::StaticClass()), AssetDataArray);

    for (const FAssetData& AssetData : AssetDataArray)
    {
        UDialogBuilderGraph* DialogGraph = Cast<UDialogBuilderGraph>(AssetData.GetAsset());
        if (DialogGraph)
        {
            for (UDialog* Dialog : DialogGraph->DialogList)
            {
                //TODO : Fix Cached ID BUG
                //if (Dialog->ID != RuntimeModule.CachedDialogID) continue;
                for (UDialogBuilderNode* Node : Dialog->AllNodes)
                {
                    if (Node)
                    {
                        if (UDialogBuilderNode_Root* RootNode = Cast<UDialogBuilderNode_Root>(Node)) continue;
                        NodeIDs.Emplace(Node->ID);
                    }
                }
            }
		}
	}
	return NodeIDs;


}

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

UDialog* UDialogBuilderFunctionLibrary::FindDialogFromNodeID(const UObject* WorldContextObject, FName NodeID)
{   
    if (UDialogComponent* DialogComp = GetDialogComponentFromTarget(UGameplayStatics::GetPlayerController(WorldContextObject, 0)))
    {
        UDialog* Dialog = DialogComp->FindDialogFromNodeID(NodeID);
        
		if (Dialog)
		{
			return Dialog;
		}
    }
    return nullptr;
}

bool UDialogBuilderFunctionLibrary::IsCurrentNodeIDMatch(const UObject* WorldContextObject, FName NodeID)
{
    UDialog* Dialog = FindDialogFromNodeID(WorldContextObject, NodeID);
    
    if (Dialog && Dialog->CurrentNode && Dialog->CurrentNode->ID == NodeID)
    {
        return true;
    }
    
    return false;
}

bool UDialogBuilderFunctionLibrary::IsDialogAtState(const UObject* WorldContextObject, FName ID, EDialogState DialogState)
{
    if (UDialogComponent* DialogComp = GetDialogComponentFromTarget(UGameplayStatics::GetPlayerController(WorldContextObject, 0)))
    {
		UDialog* Dialog = DialogComp->FindDialog(ID);

        if (Dialog && Dialog->DialogState == DialogState)
        {
            return true;
        }
    }

    return false;
}

UDialog* UDialogBuilderFunctionLibrary::GetCurrentNavigatedDialog(const UObject* WorldContextObject)
{
	if (UDialogComponent* DialogComp = GetDialogComponentFromTarget(UGameplayStatics::GetPlayerController(WorldContextObject, 0)))
	{
		return DialogComp->CurrentNavigatedDialog;
	}

    UE_LOG(LogTemp, Warning, TEXT("DialogBuilderFunctionLibrary : Current Navigated Dialog Is Null!"));
    return nullptr;
}
