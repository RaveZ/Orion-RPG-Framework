// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Task/QuestTask.h"
#include "QuestComponent.h"
#include "Quest.h"
#include "Event/OrionEvent.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderFunctionLibrary.h"
#include "QuestBuilderNode_Objective.h"
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
	if (Quest->QuestState != EQuestState::E_Active)
        return false;

    TaskState = ETaskState::E_Complete;
    K2_TaskCompleted(GetOwningController(), GetOwningPawn());
    GetQuestComponent()->OnQuestObjectiveUpdated.Broadcast(GetOwningQuest());
	GetQuestComponent()->OnQuestAutoSave.Broadcast();
	ObjectiveNode->EvaluateAllTaskCompleted();
	
	Deactivate();
	return true;
}

bool UQuestTask::FailTask()
{
	if (Quest->QuestState != EQuestState::E_Active)
		return false;
	TaskState = ETaskState::E_Fail;
    K2_TaskFailed(GetOwningController(), GetOwningPawn());
	GetQuestComponent()->OnQuestObjectiveUpdated.Broadcast(GetOwningQuest());
	GetQuestComponent()->OnQuestAutoSave.Broadcast();
	Deactivate();
	return true;
}

void UQuestTask::Deactivate()
{
	//clear tick timer handle
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle_TickTask);
	}
}

void UQuestTask::Reset()
{
	Deactivate();
	TaskState = ETaskState::E_Active;
	CurrentAmount = 0;
}

bool UQuestTask::IsTaskCompleted()
{
	return TaskState == ETaskState::E_Complete;
}

bool UQuestTask::IsTaskFailed()
{
	return TaskState == ETaskState::E_Fail;
}

void UQuestTask::BeginTask()  
{
	if (TaskState == ETaskState::E_Complete)
		return;
	TaskState = ETaskState::E_Active;
   K2_BeginTask(GetOwningController(), GetOwningPawn());

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
	return OwningController;
}

APawn* UQuestTask::GetOwningPawn() const
{
	if (QuestComponent)
		return QuestComponent->GetOwningPawn();
	return nullptr;
}

UQuestComponent* UQuestTask::GetQuestComponent() const
{
	return QuestComponent;
}

UQuestBuilderGraph* UQuestTask::GetOwningQuestGraph() const
{
	return QuestGraph;
}

UQuest* UQuestTask::GetOwningQuest() const
{
	return Quest;
}

void UQuestTask::AddAmount(int AmountToAdd)
{
	CurrentAmount = FMath::Clamp(CurrentAmount + AmountToAdd, 0, RequiredAmount);
	if (CurrentAmount >= RequiredAmount)
	{
		CompleteTask();
	}
}

FText UQuestTask::GetDescription_Implementation() const
{
    return Description.IsEmpty() ? FText::FromString("Description...") : Description;
}

FString UQuestTask::GetNodeName()
{
	return NodeName.Len() ? NodeName : GetShortTypeName(this);
}

FString UQuestTask::GetShortTypeName(UObject* Ob)
{
	if ((Ob == nullptr) || (Ob->GetClass() == nullptr))
	{
		return TEXT("None");
	}

	FString TypeDesc = Ob->GetClass()->GetName();

	if (Ob->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		
		TypeDesc =  Ob->GetClass()->GetName().LeftChop(2);
	}

	// Insert space before each capital letter (except the first character) and remove underscores
	// Do not add space if the previous character is uppercase (abbreviation)
	FString Result;
	for (int32 i = 0; i < TypeDesc.Len(); ++i)
	{
		if (TypeDesc[i] != TEXT('_'))
		{
			TCHAR Char = TypeDesc[i];
			if (i > 0 && FChar::IsUpper(Char) && !FChar::IsWhitespace(TypeDesc[i - 1]))
			{
				// Only add space if previous character is not uppercase (not abbreviation)
				if (!FChar::IsUpper(TypeDesc[i - 1]))
				{
					Result += TEXT(" ");
				}
			}
			Result += Char;
		}
	}
	return Result;
}

UWorld* UQuestTask::GetWorld() const
{
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		// If we are a CDO, we must return nullptr instead of calling Outer->GetWorld() to fool UObject::ImplementsGetWorld.
		return nullptr;
	}

	UObject* Outer = GetOuter();

	while (Outer)
	{
		UWorld* World = Outer->GetWorld();
		if (World)
		{
			return World;
		}

		Outer = Outer->GetOuter();
	}

	return nullptr;
	
}


