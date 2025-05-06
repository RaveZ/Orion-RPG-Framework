// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderFunctionLibrary.h"
#include "Quest_System_Runtime.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderNode_Root.h"
#include "Quest.h"
#include "QuestData.h"
#include "QuestComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AssetRegistry/AssetRegistryModule.h"

TArray<FName> UQuestBuilderFunctionLibrary::GetAllQuestID()
{
    TArray<FName> QuestIDs;

    // Load all UQuestBuilderGraph assets
    TArray<FAssetData> AssetDataArray;
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UQuestBuilderGraph::StaticClass()), AssetDataArray);

    for (const FAssetData& AssetData : AssetDataArray)
    {
        UQuestBuilderGraph* QuestGraph = Cast<UQuestBuilderGraph>(AssetData.GetAsset());
        if (QuestGraph)
        {
            for (UQuest* Quest : QuestGraph->QuestSystems)
            {
                QuestIDs.Add(Quest->QuestID);
            }
        }
    }

    return QuestIDs;
}

TArray<FName> UQuestBuilderFunctionLibrary::GetNodeIDFromCachedQuest()
{
	TArray<FName> NodeIDs;
    // Load all UQuestBuilderGraph assets
    TArray<FAssetData> AssetDataArray;
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UQuestBuilderGraph::StaticClass()), AssetDataArray);

    for (const FAssetData& AssetData : AssetDataArray)
    {
        UQuestBuilderGraph* QuestGraph = Cast<UQuestBuilderGraph>(AssetData.GetAsset());
        if (QuestGraph)
        {
            for (UQuest* Quest : QuestGraph->QuestSystems)
            {
                //TODO : Fix Cached QuestID BUG
                //if (Quest->QuestID != RuntimeModule.CachedQuestID) continue;
                for (UQuestBuilderNode* Node : Quest->AllNodes)
                {
                    if (Node)
                    {
                        if (UQuestBuilderNode_Root* RootNode = Cast<UQuestBuilderNode_Root>(Node)) continue;
                        NodeIDs.Emplace(Node->ID);
                    }
                }
            }
		}
	}
	return NodeIDs;


}

UQuestComponent* UQuestBuilderFunctionLibrary::GetQuestComponent(const UObject* WorldContextObject)
{
    return GetQuestComponentFromTarget(UGameplayStatics::GetPlayerController(WorldContextObject, 0));
}

UQuestComponent* UQuestBuilderFunctionLibrary::GetQuestComponentFromTarget(AActor* Target)
{
    if (!Target)
    {
        return nullptr;
    }

    if (UQuestComponent* QuestComp = Target->FindComponentByClass<UQuestComponent>())
    {
        return QuestComp;
    }

    //Quest comp may be on the controllers pawn or pawns controller
    if (APlayerController* OwningController = Cast<APlayerController>(Target))
    {
        if (OwningController->GetPawn())
        {
            return OwningController->GetPawn()->FindComponentByClass<UQuestComponent>();
        }
    }

    if (APawn* OwningPawn = Cast<APawn>(Target))
    {
        if (OwningPawn->GetController())
        {
            return OwningPawn->GetController()->FindComponentByClass<UQuestComponent>();
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("QuestBuilderFunctionLibrary : Current Component Not Found!"));
    return nullptr;
}

UQuest* UQuestBuilderFunctionLibrary::FindQuestFromNodeID(const UObject* WorldContextObject, FName NodeID)
{   
    if (UQuestComponent* QuestComp = GetQuestComponentFromTarget(UGameplayStatics::GetPlayerController(WorldContextObject, 0)))
    {
        UQuest* Quest = QuestComp->FindQuestFromNodeID(NodeID);
        
		if (Quest)
		{
			return Quest;
		}
    }
    return nullptr;
}

bool UQuestBuilderFunctionLibrary::IsCurrentNodeIDMatch(const UObject* WorldContextObject, FName NodeID)
{
    UQuest* Quest = FindQuestFromNodeID(WorldContextObject, NodeID);
    
    if (Quest && Quest->CurrentNode && Quest->CurrentNode->ID == NodeID)
    {
        return true;
    }
    
    return false;
}

bool UQuestBuilderFunctionLibrary::IsQuestAtState(const UObject* WorldContextObject, FName QuestID, EQuestState QuestState)
{
    if (UQuestComponent* QuestComp = GetQuestComponentFromTarget(UGameplayStatics::GetPlayerController(WorldContextObject, 0)))
    {
		UQuest* Quest = QuestComp->FindQuest(QuestID);

        if (Quest && Quest->QuestState == QuestState)
        {
            return true;
        }
    }

    return false;
}

UQuest* UQuestBuilderFunctionLibrary::GetCurrentNavigatedQuest(const UObject* WorldContextObject)
{
	if (UQuestComponent* QuestComp = GetQuestComponentFromTarget(UGameplayStatics::GetPlayerController(WorldContextObject, 0)))
	{
		return QuestComp->CurrentNavigatedQuest;
	}

    UE_LOG(LogTemp, Warning, TEXT("QuestBuilderFunctionLibrary : Current Navigated Quest Is Null!"));
    return nullptr;
}
