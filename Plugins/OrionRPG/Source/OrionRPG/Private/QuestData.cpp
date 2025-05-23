// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "QuestData.h"
#include "OrionRPG.h"
#include "Quest.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderNode_Root.h"
#include "AssetRegistry/AssetRegistryModule.h"

TArray<FName> UQuestHandle::GetAllQuestID()
{
	TArray<FName> QuestIDs;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> QuestGraphDataArray;
	AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UQuestBuilderGraph::StaticClass()), QuestGraphDataArray);

	for (const FAssetData& AssetData : QuestGraphDataArray)
	{
		UQuestBuilderGraph* QuestGraphAsset = Cast<UQuestBuilderGraph>(AssetData.GetAsset());
		if (QuestGraphAsset)
		{
			for (UQuest* QuestObj : QuestGraphAsset->QuestList)
			{
				QuestIDs.Add(QuestObj->ID);
			}
		}
	}

	return QuestIDs;
}

TArray<FName> UQuestHandle::GetAllNodeID()
{
	TArray<FName> NodeIDs;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> QuestGraphDataArray;
	AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UQuestBuilderGraph::StaticClass()), QuestGraphDataArray);

	for (const FAssetData& AssetData : QuestGraphDataArray)
	{
		UQuestBuilderGraph* QuestGraphAsset = Cast<UQuestBuilderGraph>(AssetData.GetAsset());
		if (QuestGraphAsset)
		{
			for (UQuest* QuestObj : QuestGraphAsset->QuestList)
			{
				if (QuestObj->ID != QuestID) continue;
				for (UQuestBuilderNode* Node : QuestObj->AllNodes)
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

FQuestDataHandle::FQuestDataHandle()
{
}

void FQuestDataHandle::PostSerialize(const FArchive& Ar)
{
	if (QuestDataHandle)
	{
		QuestID = QuestDataHandle->QuestID;
		if (QuestID != NAME_None)
		{
			NodeID = QuestDataHandle->NodeID;
		}
		else
		{
			NodeID = NAME_None;
		}
	}
	else
	{
	}
	
}