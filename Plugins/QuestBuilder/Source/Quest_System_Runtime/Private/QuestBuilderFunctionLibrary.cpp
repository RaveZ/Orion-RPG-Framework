// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderFunctionLibrary.h"
#include "Quest_System_Runtime.h"
#include "QuestSystemGraph.h"
#include "QuestSystemGraphNode.h"
#include "QuestSystemGraphNode_Root.h"
#include "QuestSystem.h"
#include "QuestData.h"
#include "QuestComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AssetRegistry/AssetRegistryModule.h"

TArray<FName> UQuestBuilderFunctionLibrary::GetAllQuestID()
{
    TArray<FName> QuestIDs;

    // Load all UQuestSystemGraph assets
    TArray<FAssetData> AssetDataArray;
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UQuestSystemGraph::StaticClass()), AssetDataArray);

    for (const FAssetData& AssetData : AssetDataArray)
    {
        UQuestSystemGraph* QuestGraph = Cast<UQuestSystemGraph>(AssetData.GetAsset());
        if (QuestGraph)
        {
            for (UQuestSystem* Quest : QuestGraph->QuestSystems)
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
    // Load all UQuestSystemGraph assets
    TArray<FAssetData> AssetDataArray;
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UQuestSystemGraph::StaticClass()), AssetDataArray);

    for (const FAssetData& AssetData : AssetDataArray)
    {
        UQuestSystemGraph* QuestGraph = Cast<UQuestSystemGraph>(AssetData.GetAsset());
        if (QuestGraph)
        {
            for (UQuestSystem* Quest : QuestGraph->QuestSystems)
            {
                //TODO : Fix Cached QuestID BUG
                //if (Quest->QuestID != RuntimeModule.CachedQuestID) continue;
                for (UQuestSystemGraphNode* Node : Quest->AllNodes)
                {
                    if (Node)
                    {
                        if (UQuestSystemGraphNode_Root* RootNode = Cast<UQuestSystemGraphNode_Root>(Node)) continue;
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

    return nullptr;
}
