// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Decorator/HasQuestReachedCheckpoint.h"
#include "QuestBuilderNode_Objective.h"
#include "QuestBuilderNode_Checkpoint.h"
#include "QuestComponent.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Quest.h"
#include "QuestBuilderGraph.h"

UHasQuestReachedCheckpoint::UHasQuestReachedCheckpoint()
{
    bUseQuestID = true;
	bUseNodeID = true;
}

bool UHasQuestReachedCheckpoint::PerformConditionCheck_Implementation(AController* OwnerController, APawn* ControlledPawn) const
{
    if (GetQuestComponent())
    {
		if (UQuest* FoundedQuest = GetQuestComponent()->QuestMap.FindRef(QuestID))
		{
			return FoundedQuest->VisitedNodeIDs.Contains(NodeID);
		}
    }
    return false;
}

FString UHasQuestReachedCheckpoint::GetNodeDisplayText_Implementation() const
{
    return FString::Printf(TEXT("In Quest: %s \nIn Checkpoint: %s"), *QuestID.ToString(), *NodeID.ToString());
}

TArray<FName> UHasQuestReachedCheckpoint::GetAllNodeID()
{
	TArray<FName> NodeIDs;
	if (bUseQuestID)
	{
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
							if (UQuestBuilderNode_Checkpoint* CheckpointNode = Cast<UQuestBuilderNode_Checkpoint>(Node))
							{
								NodeIDs.Emplace(Node->ID);
							}
						}
					}
				}
			}
		}
	}
	return NodeIDs;
}

