// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderFunctionLibrary.h"
#include "Quest_System_Runtime.h"
#include "QuestSystemGraph.h"
#include "QuestSystemGraphNode.h"
#include "QuestSystemGraphNode_Root.h"
#include "QuestSystem.h"
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
	FQuest_System_RuntimeModule& RuntimeModule = FModuleManager::GetModuleChecked<FQuest_System_RuntimeModule>(TEXT("Quest_System_Runtime"));
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
                if (Quest->QuestID != RuntimeModule.CachedQuestID) continue;
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
