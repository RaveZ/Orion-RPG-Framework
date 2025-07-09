// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Decorator/HasQuestEverReachedCheckpoint.h"
#include "QuestBuilderNode_Objective.h"
#include "QuestBuilderFunctionLibrary.h"
#include "QuestBuilderNode_Checkpoint.h"
#include "QuestComponent.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Quest.h"
#include "QuestBuilderGraph.h"

UHasQuestEverReachedCheckpoint::UHasQuestEverReachedCheckpoint()
{
    bUseQuestTag = true;
    bUseNodeTag = true;
}

bool UHasQuestEverReachedCheckpoint::PerformConditionCheck_Implementation(AController* OwnerController, APawn* ControlledPawn) const
{
    FName QuestID = QuestTag.GetTagName();
    FName NodeID = NodeTag.GetTagName();
    if (GetWorld())
    {
        if (UQuestComponent* QuestComp = UQuestBuilderFunctionLibrary::GetQuestComponent(GetWorld()))
        {
            if (UQuest* FoundedQuest = QuestComp->QuestMap.FindRef(QuestID))
            {
                return FoundedQuest->VisitedNodeIDs.Contains(NodeID);
            }
        }
    }
		
    return false;
}

FString UHasQuestEverReachedCheckpoint::GetNodeDisplayText_Implementation() const
{
    return FString::Printf(TEXT("In Quest: %s \nIn Checkpoint: %s"), *GetShortTag(QuestTag), *GetShortTag(NodeTag));
}
