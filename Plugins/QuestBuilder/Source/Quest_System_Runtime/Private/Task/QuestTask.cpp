// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Task/QuestTask.h"
#include "QuestComponent.h"
#include "Quest.h"
#include "Event/OrionEvent.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"
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
	if (Quest == nullptr)
        return false;

    TaskState = ETaskState::E_Complete;
    K2_TaskCompleted(GetOwningController(), GetOwningPawn());
	GetQuestComponent()->UpdateQuest();
    GetQuestComponent()->OnQuestObjectiveUpdated.Broadcast();

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

bool UQuestTask::IsTaskCompleted()
{
	return TaskState == ETaskState::E_Complete;
}

void UQuestTask::BeginTask()  
{  
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

UQuestBuilderGraph* UQuestTask::GetOwningQuestGraph() const
{
	if (!QuestGraph)
		return nullptr;

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
	GetQuestComponent()->OnQuestObjectiveUpdated.Broadcast();
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
	FString Result;
	for (int32 i = 0; i < TypeDesc.Len(); ++i)
	{
		if (TypeDesc[i] != TEXT('_'))
		{
			TCHAR Char = TypeDesc[i];
			if (i > 0 && FChar::IsUpper(Char) && !FChar::IsWhitespace(TypeDesc[i - 1]))
			{
				Result += TEXT(" ");
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


