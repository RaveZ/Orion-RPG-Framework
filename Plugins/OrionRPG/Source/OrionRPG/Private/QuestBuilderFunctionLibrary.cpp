// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderFunctionLibrary.h"
#include "OrionRPG.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderNode_Objective.h"
#include "QuestBuilderNode_Root.h"
#include "Quest.h"
#include "QuestData.h"
#include "QuestComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AssetRegistry/AssetRegistryModule.h"

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

UQuest* UQuestBuilderFunctionLibrary::FindQuest(const UObject* WorldContextObject, FGameplayTag QuestTag)
{   
    if (UQuestComponent* QuestComp = GetQuestComponentFromTarget(UGameplayStatics::GetPlayerController(WorldContextObject, 0)))
    {
        return QuestComp->FindQuest(QuestTag);
    }
    return nullptr;
}

bool UQuestBuilderFunctionLibrary::CanProgressQuestObjective(const UObject* WorldContextObject, FGameplayTag QuestTag, FGameplayTag NodeTag)
{
    if (UQuestComponent* QuestComp = GetQuestComponentFromTarget(UGameplayStatics::GetPlayerController(WorldContextObject, 0)))
    {
        UQuest* FoundedQuest = QuestComp->FindQuest(QuestTag);
        UQuestBuilderNode* FoundedNode = FoundedQuest->NodeMap.FindRef(NodeTag.GetTagName());
        if (FoundedQuest && FoundedQuest->CurrentNodes.Contains(FoundedNode) && FoundedQuest->QuestState == EQuestState::E_Active)
        {
            if (UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(FoundedNode))
            {
                return true;
            }
        }
    }
    return false;
}

bool UQuestBuilderFunctionLibrary::IsQuestAtState(const UObject* WorldContextObject, FGameplayTag QuestTag, EQuestState QuestState)
{
    if (UQuestComponent* QuestComp = GetQuestComponentFromTarget(UGameplayStatics::GetPlayerController(WorldContextObject, 0)))
    {
		return QuestComp->IsQuestAtState(QuestTag, QuestState);
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

bool UQuestBuilderFunctionLibrary::IsCurrentlyNavigatedObjective(const UObject* WorldContextObject, FName QuestID, FName NodeID)
{
    if (UQuestComponent* QuestComp = GetQuestComponentFromTarget(UGameplayStatics::GetPlayerController(WorldContextObject, 0)))
    {
        UQuest* CurrentNavigatedQuest = QuestComp->CurrentNavigatedQuest;
        UQuest* FoundedQuest =  QuestComp->QuestMap.FindRef(QuestID);
        UQuestBuilderNode* FoundedNode =  FoundedQuest ? FoundedQuest->NodeMap.FindRef(NodeID) : nullptr;

        if ((CurrentNavigatedQuest && FoundedQuest && CurrentNavigatedQuest == FoundedQuest) &&
            (FoundedQuest->QuestState == EQuestState::E_Active) && 
            (FoundedNode && CurrentNavigatedQuest->CurrentNodes.Contains(FoundedNode)))
        {
            UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(FoundedNode);
            if (ObjectiveNode && !ObjectiveNode->IsAllTaskCompleted())
            {
                return true;
            }
        }
    }
    return false;
}

bool UQuestBuilderFunctionLibrary::IsObjectiveCompleted(const UObject* WorldContextObject, UQuestBuilderNode* QuestNode)
{
    if ( UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(QuestNode))
    {
        return ObjectiveNode->IsAllTaskCompleted();
    }
    return false;
}

void UQuestBuilderFunctionLibrary::AutoNavigateQuest(const UObject* WorldContextObject)
{
    if (UQuestComponent* QuestComp = GetQuestComponentFromTarget(UGameplayStatics::GetPlayerController(WorldContextObject, 0)))
    {
        QuestComp->AutoNavigateQuest();
    }
}
