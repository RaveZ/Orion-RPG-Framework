// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Decorator/IsObjectiveCompleted.h"
#include "QuestBuilderNode_Objective.h"
#include "QuestComponent.h"
#include "Quest.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "QuestBuilderGraph.h"

UIsObjectiveCompleted::UIsObjectiveCompleted()
{
    bUseQuestID = true;
	bUseNodeID = true;
}

bool UIsObjectiveCompleted::PerformConditionCheck_Implementation(AController* OwnerController, APawn* ControlledPawn) const
{

    for (const auto& It : GetQuestComponent()->QuestMap)
    {
        FName Key = It.Key;
        UQuest* QuestObj = It.Value;

        if (UQuestBuilderNode_Objective* ObjectiveNode =  Cast<UQuestBuilderNode_Objective>(QuestObj->NodeMap.FindRef(NodeID)))
        {
            return ObjectiveNode->IsAllTaskCompleted();
        }
    }
    return false;
}

FString UIsObjectiveCompleted::GetNodeDisplayText_Implementation() const
{
    return FString::Printf(TEXT("In Quest: %s \nIn Objective: %s"), *QuestID.ToString(), *NodeID.ToString());
}

TArray<FName> UIsObjectiveCompleted::GetAllNodeID()
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
							if (UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(Node))
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
