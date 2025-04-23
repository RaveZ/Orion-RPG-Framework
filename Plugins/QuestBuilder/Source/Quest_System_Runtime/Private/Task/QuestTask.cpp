// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "Task/QuestTask.h"
#include "QuestComponent.h"
#include "QuestSystem.h"
#include "Event/QuestEvent.h"
#include "QuestSystemGraph.h"
#include "QuestSystemGraphNode.h"
#include "QuestSystemGraphNode_Objective.h"
#include <TimerManager.h>

UQuestTask::UQuestTask()
{
	bOptional = false;
	bHidden = false;
	bIsTaskTickable = false;
	TickInterval = 0.1f; 
	RequiredAmount = 1;
}

bool UQuestTask::CompleteTask()
{	
	if (QuestSystem == nullptr)
        return false;

    TaskState = ETaskState::E_Complete;
    K2_TaskCompleted();
	GetQuestComponent()->UpdateQuest();
    GetQuestComponent()->OnQuestObjectiveUpdated.Broadcast();

	//clear tick timer handle
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle_TickTask);
	}
	return true;
}

void UQuestTask::BeginTask()  
{  
   K2_BeginTask();  

   if (GetQuestComponent() && bIsTaskTickable)
   {  
       if (TickInterval > 0.f)  
       {  
		   UWorld* World = GetWorld();
           if (World)  
           {  
               World->GetTimerManager().SetTimer(TimerHandle_TickTask, this, &UQuestTask::TickTask, TickInterval, true);  
           }  
           else  
           {  
               UE_LOG(LogTemp, Warning, TEXT("BeginTask: Failed to get UWorld instance."));  
           }  
       }  

       // Fire the first tick off after BeginTask since begin task will usually init things that TickTask may need  
       TickTask();  
   }  
}

void UQuestTask::TickTask_Implementation()
{
}

AController* UQuestTask::GetOwningController() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->OwningController;
}

APawn* UQuestTask::GetOwningPawn() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->OwningPawn;
}

UQuestComponent* UQuestTask::GetQuestComponent() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph->QuestComponent;
}

UQuestSystemGraph* UQuestTask::GetOwningQuestGraph() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph;
}

UQuestSystem* UQuestTask::GetOwningQuestSystem() const
{
	return QuestSystem;
}



void UQuestTask::AddAmount(int AmountToAdd)
{
	CurrentAmount = FMath::Clamp(CurrentAmount + AmountToAdd, 0, RequiredAmount);
	if (CurrentAmount >= RequiredAmount)
	{
		CompleteTask();
	}
	GetQuestComponent()->OnQuestObjectiveUpdated.Broadcast();
}

FText UQuestTask::GetDescription_Implementation() const
{
	return Description;
}

UWorld* UQuestTask::GetWorld() const
{
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		// If we are a CDO, we must return nullptr instead of calling Outer->GetWorld() to fool UObject::ImplementsGetWorld.
		return nullptr;
	}

	if (GetQuestComponent())
	{
		return GetQuestComponent()->GetWorld();
	}

	return nullptr;
	
}


