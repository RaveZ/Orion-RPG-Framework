// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderNode_Objective.h"
#include "Task/QuestTask.h"
#include "Quest.h"
#include "QuestComponent.h"


#define LOCTEXT_NAMESPACE "QuestSystemGraphNode_Objective"


UQuestBuilderNode_Objective::UQuestBuilderNode_Objective()
{
    bOptional = false;
    bHidden = false;
    bIsObjectiveTickable = false;
    TickInterval = 0.1f;
    RequiredAmount = 1;
}

void UQuestBuilderNode_Objective::Initialize(bool bLaunchEventOnLoad)
{
    if (IsObjectiveCompleted())
    {
        EvaluateNextNode();
    }
    else
    {
        Super::Initialize(bLaunchEventOnLoad);
    }
}

void UQuestBuilderNode_Objective::BeginNode()
{
    Super::BeginNode();
	BeginObjective();
    GetQuestComponent()->OnQuestAutoSave.Broadcast();
}

void UQuestBuilderNode_Objective::Deinitialize()
{
	Super::Deinitialize();
}

void UQuestBuilderNode_Objective::Reset()
{
    Super::Reset();
    ObjectiveState = EObjectiveState::E_Active;
    CurrentAmount = 0;
	DeactivateTick();
}

bool UQuestBuilderNode_Objective::IsObjectiveCompleted()
{
	return ObjectiveState == EObjectiveState::E_Completed;
}

bool UQuestBuilderNode_Objective::IsObjectiveFailed()
{
    return ObjectiveState == EObjectiveState::E_Failed;
}

bool UQuestBuilderNode_Objective::IsObjectiveActive()
{
    return ObjectiveState == EObjectiveState::E_Active;
}

void UQuestBuilderNode_Objective::TickObjective_Implementation()
{
}

void UQuestBuilderNode_Objective::AddAmount(int AmountToAdd)
{
    CurrentAmount = FMath::Clamp(CurrentAmount + AmountToAdd, 0, RequiredAmount);
    if (CurrentAmount >= RequiredAmount)
    {
        CompleteObjective();
    }
}

void UQuestBuilderNode_Objective::BeginObjective()
{
    if (ObjectiveState == EObjectiveState::E_Completed)
        return;
    ObjectiveState = EObjectiveState::E_Active;
    K2_BeginObjective(GetOwningController(), GetOwningPawn());

    if (GetQuestComponent() && bIsObjectiveTickable)
    {
        if (TickInterval > 0.f)
        {
            UWorld* World = GetWorld();
            if (World)
            {
                World->GetTimerManager().SetTimer(TimerHandle_TickObjective, this, &UQuestBuilderNode_Objective::TickObjective, TickInterval, true);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("BeginTask: Failed to get UWorld instance."));
            }
        }

        // Fire the first tick off after BeginTask since begin task will usually init things that TickTask may need  
        TickObjective();
    }
}

bool UQuestBuilderNode_Objective::CompleteObjective()
{
    if (Quest->QuestState != EQuestState::E_Active)
        return false;

    ObjectiveState = EObjectiveState::E_Completed;
    K2_ObjectiveCompleted(GetOwningController(), GetOwningPawn());
    GetQuestComponent()->OnQuestObjectiveUpdated.Broadcast(GetOwningQuest());
    GetQuestComponent()->OnQuestAutoSave.Broadcast();
    GetQuestComponent()->OnQuestUpdated.Broadcast();

    Deinitialize();
    DeactivateTick();
    return true;
}

bool UQuestBuilderNode_Objective::FailObjective()
{
    if (Quest->QuestState != EQuestState::E_Active)
        return false;
    ObjectiveState = EObjectiveState::E_Failed;
    K2_ObjectiveFailed(GetOwningController(), GetOwningPawn());
    GetQuestComponent()->OnQuestObjectiveUpdated.Broadcast(GetOwningQuest());
    GetQuestComponent()->OnQuestAutoSave.Broadcast();
    GetQuestComponent()->OnQuestUpdated.Broadcast();

    DeactivateTick();
    return true;
}

void UQuestBuilderNode_Objective::DeactivateTick()
{
    //clear tick timer handle
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TimerHandle_TickObjective);
    }
}


void UQuestBuilderNode_Objective::EvaluateNextNode()
{
    if (IsObjectiveCompleted())
    {
        Super::EvaluateNextNode();
    }
}


#if WITH_EDITOR
FText UQuestBuilderNode_Objective::GetNodeTitle() const
{
    return FText::FromString("Objective Node");
}

void UQuestBuilderNode_Objective::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}

FText UQuestBuilderNode_Objective::GetNodeDescription() const
{
    /*FString TaskDesc;

    for (const UQuestTask* Task : Tasks)
    {
        if (Task)
        {
            if(!TaskDesc.IsEmpty()) TaskDesc += TEXT("\n");
            TaskDesc += Task->GetDescription().ToString();
        }
        
    }

    return FText::FromString(TaskDesc);*/
    return Description;
}
#endif

#undef LOCTEXT_NAMESPACE