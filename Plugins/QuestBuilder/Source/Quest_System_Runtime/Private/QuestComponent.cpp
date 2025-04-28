// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "QuestComponent.h"
#include "QuestSystem.h"
#include "Condition/QuestCondition.h"
#include "Event/QuestEvent.h"
#include "Task/QuestTask.h"
#include "UObject/UObjectIterator.h"
#include "QuestSaveObject.h"
#include "QuestSystemGraph.h"
#include "QuestSystemGraphNode_State.h"
#include "QuestSystemGraphNode_Root.h"
#include "QuestSystemGraphNode_Objective.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Controller.h"
#include "QuestData.h"




#define LOCTEXT_NAMESPACE "QuestComponent"
// Sets default values for this component's properties
UQuestComponent::UQuestComponent()
{
	SetIsReplicatedByDefault(true);
	SetComponentTickEnabled(true);
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UQuestComponent::BeginPlay()
{
	Super::BeginPlay();
	QuestSystemMap.Empty();
	bQuestLoaded = false;
	//Set Dependency
	OwningController = Cast<AController>(GetOwner());

	OnQuestObjectiveUpdated.AddDynamic(this, &UQuestComponent::QuestObjectiveUpdated);
	OnStartNavigateQuest.AddDynamic(this, &UQuestComponent::StartNavigatingQuest);
	OnQuestFailed.AddDynamic(this, &UQuestComponent::QuestFailed);
	OnQuestCompleted.AddDynamic(this, &UQuestComponent::QuestCompleted);
	OnQuestAdded.AddDynamic(this, &UQuestComponent::QuestAdded);
	OnBeginSave.AddDynamic(this, &UQuestComponent::BeginSave);
	OnSaveComplete.AddDynamic(this, &UQuestComponent::SaveComplete);
	OnBeginLoad.AddDynamic(this, &UQuestComponent::BeginLoad);
	OnLoadComplete.AddDynamic(this, &UQuestComponent::LoadComplete);

#if WITH_ORION_COMBAT_FRAMEWORK

#else

#endif

}


// Called every frame
void UQuestComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UQuestComponent::BeginQuest(UQuestSystemGraph* QuestAsset)
{
	if (QuestAsset == nullptr)
	{
		return false;
	}

	CacheQuestGraphDependencies(QuestAsset);
	EvaluateQuestSaveLoadState(QuestAsset);
	bool bSuccess = true;

	for (UQuestSystem* QuestSystem : QuestAsset->QuestSystems)
	{
		if (!ShouldUpdateQuest(QuestSystem))
			continue;
		EvaluateQuestCondition(QuestSystem);
	}

	return bSuccess;
}

bool UQuestComponent::BeginMultipleQuest(TArray<UQuestSystemGraph*> QuestAssets)
{
	if (QuestAssets.IsEmpty())
	{
		return false;
	}


	for (UQuestSystemGraph* QuestAsset : QuestAssets)
	{
		BeginQuest(QuestAsset);
	}
	bool bSuccess = true;
	return bSuccess;
}

void UQuestComponent::CacheQuestGraphDependencies(UQuestSystemGraph* QuestGraph)
{
	if (QuestGraph)
	{
		QuestGraph->OwningController = OwningController;
		QuestGraph->QuestComponent = this;
		QuestGraph->OwningPawn = GetOwningPawn();

		//Add QuestGraph to array
		QuestGraphs.Add(QuestGraph);

		for (UQuestSystem* Quest : QuestGraph->QuestSystems)
		{
			QuestSystemMap.Emplace(Quest->QuestID, Quest);
		}
	}
}

void UQuestComponent::EvaluateQuestSaveLoadState(UQuestSystemGraph* QuestGraph)
{
	for (UQuestSystem* Quest : QuestGraph->QuestSystems)
	{
		//check if quest has been loaded
		if (!bQuestLoaded)
		{
			Quest->QuestState = EQuestState::E_Locked; // all quest state is locked by default
			Quest->CurrentNode = nullptr;
			Quest->VisitedNodeIDs.Empty();
		}
	}
}

void UQuestComponent::ActivateQuest(FName QuestID)
{
	if (UQuestSystem* QuestSystem = QuestSystemMap.FindRef(QuestID))
	{
		QuestSystem->QuestState = EQuestState::E_Active;
		if (QuestSystem->CurrentNode != nullptr)
		{
			BeginNode(QuestSystem);
		}
		else if (QuestSystem->CurrentNode == nullptr)
		{
			OnQuestAdded.Broadcast(QuestSystem);
			RestartQuest(QuestSystem->QuestID);
		}
	}
}

void UQuestComponent::RestartQuest(FName QuestID, FName NodeID, bool bStartFromNodeID)
{
	if (UQuestSystem* QuestSystem = QuestSystemMap.FindRef(QuestID))
	{
		QuestSystem->QuestState = EQuestState::E_Active;


		bool bNodeFound = false;
		if (NodeID.IsNone())
			bStartFromNodeID = false;

		if (bStartFromNodeID)
		{	
			if (UQuestSystemGraphNode* FoundedNode = QuestSystem->NodeMap.FindRef(NodeID))
			{
				bNodeFound = true;
				//reset visited node children where the node started from
				bool bCanDelete = false;
				for (int i = 0; i < QuestSystem->VisitedNodeIDs.Num(); i++)
				{
					if (QuestSystem->VisitedNodeIDs[i] == NodeID)
					{
						bCanDelete = true;
					}
					if (bCanDelete)
					{
						QuestSystem->VisitedNodeIDs.RemoveAt(i);
						i--;//ensure the next element not skipped
					}
				}

				//reset task data from the founded node
				TArray<UQuestSystemGraphNode*> NodeStack;
				NodeStack.Add(FoundedNode);
				while (NodeStack.Num() > 0)
				{
					UQuestSystemGraphNode* CurrentNodeStack = NodeStack.Pop();
					if (UQuestSystemGraphNode_Objective* ObjectiveNode = Cast<UQuestSystemGraphNode_Objective>(CurrentNodeStack))
					{
						for (UQuestTask* Task : ObjectiveNode->Tasks)
						{
							if (Task)
							{
								Task->TaskState = ETaskState::E_Active;
								Task->CurrentAmount = 0;
							}
						}
					}
					for (UQuestEvent* Event : CurrentNodeStack->Events)
					{
						if (Event)
						{
							Event->StartLaunched = false;
							Event->EndLaunched = false;
						}
					}
					// Traverse child node
					for (int i = 0; i < CurrentNodeStack->ChildrenNodes.Num(); i++)
					{
						NodeStack.Add(CurrentNodeStack->ChildrenNodes[i]);
					}
				}

				QuestSystem->CurrentNode = FoundedNode;
			}
		}
		

		if(!bNodeFound)
		{
			//reset all task data
			for (UQuestSystemGraphNode* Node : QuestSystem->AllNodes)
			{
				if (UQuestSystemGraphNode_Objective* ObjectiveNode = Cast<UQuestSystemGraphNode_Objective>(Node))
				{
					for (UQuestTask* Task : ObjectiveNode->Tasks)
					{
						if (Task)
						{
							Task->TaskState = ETaskState::E_Active;
							Task->CurrentAmount = 0;
						}
						
					}
				}
				for (UQuestEvent* Event : Node->Events)
				{
					if (Event)
					{
						Event->StartLaunched = false;
						Event->EndLaunched = false;
					}
				}
			}
			QuestSystem->VisitedNodeIDs.Empty();
			QuestSystem->CurrentNode = QuestSystem->RootNodes[0];
		}
		

		BeginNode(QuestSystem);
	}
}

void UQuestComponent::FailQuest(FName QuestID)
{
	if (UQuestSystem* QuestSystem = QuestSystemMap.FindRef(QuestID))
	{
		if (QuestSystem->QuestState != EQuestState::E_Fail)
		{
			QuestSystem->QuestState = EQuestState::E_Fail;
			OnQuestFailed.Broadcast(QuestSystem);
		}
		QuestSystem->CurrentNode = nullptr;

	}
}

void UQuestComponent::CompleteQuest(FName QuestID)
{
	if (UQuestSystem* QuestSystem = QuestSystemMap.FindRef(QuestID))
	{
		if (QuestSystem->QuestState != EQuestState::E_Complete)
		{
			QuestSystem->QuestState = EQuestState::E_Complete;
			OnQuestCompleted.Broadcast(QuestSystem);
		}
		QuestSystem->CurrentNode = nullptr;

	}
}

void UQuestComponent::StartNavigateQuest(UQuestSystem* InQuest)
{
	if (InQuest)
	{
		CurrentNavigatedQuest = InQuest;
		OnStartNavigateQuest.Broadcast(InQuest);
	}
}


APawn* UQuestComponent::GetOwningPawn() const
{
	if (OwningController)
	{
		return OwningController->GetPawn();
	}
	return nullptr;
}

UQuestSystem* UQuestComponent::FindQuestFromNodeID(FName NodeIDParam) const
{
	UQuestSystem* FoundedQuest = nullptr;
	for (const auto& It : QuestSystemMap)
	{
		FName Key = It.Key;
		UQuestSystem* QuestSystem = It.Value;

		if (UQuestSystemGraphNode* QuestNode = QuestSystem->NodeMap.FindRef(NodeIDParam))
		{
			FoundedQuest = QuestSystem;
		}
	}

	return FoundedQuest;

}

bool UQuestComponent::IsQuestActiveFromNodeID(FName NodeID) const
{
	if (UQuestSystem* FoundedQuest = FindQuestFromNodeID(NodeID))
	{
		if (FoundedQuest->QuestState == EQuestState::E_Active)
		{
			return true;
		}
	}

	return false;
}

bool UQuestComponent::IsQuestActiveFromQuestID(FName QuestID) const
{
	if (UQuestSystem* FoundedQuest = QuestSystemMap.FindRef(QuestID))
	{
		if (FoundedQuest->QuestState == EQuestState::E_Active)
		{
			return true;
		}
	}

	return false;
}

void UQuestComponent::UpdateQuest()
{
	for (const auto& It : QuestSystemMap)
	{
		FName Key = It.Key;
		UQuestSystem* QuestSystem = It.Value;
		
		if (!ShouldUpdateQuest(QuestSystem))
			continue;

		EvaluateQuestCondition(QuestSystem);
		if (QuestSystem->CurrentNode != nullptr)
		{
			if (UQuestSystemGraphNode_Objective* ObjectiveNode = Cast<UQuestSystemGraphNode_Objective>(QuestSystem->CurrentNode))
			{
				if (ObjectiveNode->IsAllTaskCompleted())
				{
					//evaluate next node condition
					EvaluateNextNodeCondition(QuestSystem);
				}
			}
		}
		else if (QuestSystem->QuestState == EQuestState::E_Active && QuestSystem->CurrentNode == nullptr)
		{
			RestartQuest(QuestSystem->QuestID);
		}
		
	}
}

void UQuestComponent::EvaluateNextNodeCondition(UQuestSystem* QuestParam)
{
	UQuestSystemGraphNode* NextNode = nullptr;
	for (UQuestSystemGraphNode* Node : QuestParam->CurrentNode->ChildrenNodes)
	{
		if (NextNode == nullptr && Node->Conditions.IsEmpty())
		{
			NextNode = Node;
		}
		else if (!Node->Conditions.IsEmpty())
		{
			int SumConditionMet = 0;
			for (UQuestCondition* Condition : Node->Conditions)
			{
				if (Condition && Condition->IsConditionMet())
				{
					SumConditionMet++;
				}
			}
			if (SumConditionMet >= Node->Conditions.Num())
			{
				NextNode = Node;
				//break loop to ensure the met condition doesn't get overrided
				break;
			}
		}
	}

	if (NextNode)
	{
		//End of a Node, Launch Event with the End and Both Type
		for (UQuestEvent* Event : QuestParam->CurrentNode->Events)
		{
			if (Event)
			{
				if (Event->EventRunType == EEventRunType::E_Both || Event->EventRunType == EEventRunType::E_End)
				{
					if (!Event->EndLaunched)
					{
						Event->BeginEvent();
						Event->EndLaunched = true;
					}
				}
			}
			
		}
		//Go to next node if exist or condition met
		GoToNextNode(QuestParam, NextNode);

	}
	
}

void UQuestComponent::EvaluateQuestCondition(UQuestSystem* QuestParam)
{
	int SumConditionMet = 0;
	for (UQuestCondition* Condition : QuestParam->Conditions)
	{
		if (Condition && Condition->IsConditionMet())
		{
			SumConditionMet++;
		}
	}
	EvaluateQuestState(QuestParam, SumConditionMet);
	
}

void UQuestComponent::EvaluateQuestState(UQuestSystem* QuestParam, int SumConditionMet)
{
	if (QuestParam->QuestState == EQuestState::E_Complete || QuestParam->QuestState == EQuestState::E_Fail)
		return;
	if (QuestParam->bAutoActivateQuest)
	{
		ActivateQuest(QuestParam->QuestID);
		return;
	}

	switch (QuestParam->QuestState)
	{
		case EQuestState::E_Active:
			ActivateQuest(QuestParam->QuestID);
			return;
		case EQuestState::E_Locked:
			if (SumConditionMet >= QuestParam->Conditions.Num())
			{
				if (QuestParam->bUnlockQuest)
				{
					QuestParam->QuestState = EQuestState::E_Unlocked;
				}
				else
				{
					ActivateQuest(QuestParam->QuestID);
				}
			}
			return;
		case EQuestState::E_Unlocked:

			return;
	}
}

void UQuestComponent::GoToNextNode(UQuestSystem* QuestParam, UQuestSystemGraphNode* NodeParam)
{
	QuestParam->CurrentNode = NodeParam;
	BeginNode(QuestParam);
}

void UQuestComponent::BeginNode(UQuestSystem* QuestParam)  
{  
   if (QuestParam->CurrentNode == nullptr)  
   {  
       UE_LOG(LogTemp, Warning, TEXT("BeginNode: CurrentNode is null for QuestID: %s"), *QuestParam->QuestID.ToString());  
       return;  
   }  

   //add visited node
   QuestParam->VisitedNodeIDs.AddUnique(QuestParam->CurrentNode->ID);

   if (UQuestSystemGraphNode_Root* RootNode = Cast<UQuestSystemGraphNode_Root>(QuestParam->CurrentNode))
   {
	   EvaluateNextNodeCondition(QuestParam);
	   return;
   }

   //Start of the Node, Launch Event with the Start and Both Type
   for (UQuestEvent* Event : QuestParam->CurrentNode->Events)
   {
	   if (Event)
	   {
		   if (Event->EventRunType == EEventRunType::E_Both || Event->EventRunType == EEventRunType::E_Start)
		   {
			   if (!Event->StartLaunched)
			   {
				   Event->BeginEvent();
				   Event->StartLaunched = true;
			   }
		   }
	   }
   }

   //Begin Node Functionality
   if (UQuestSystemGraphNode_Objective* ObjectiveNode = Cast<UQuestSystemGraphNode_Objective>(QuestParam->CurrentNode))
   {
	   for (UQuestTask* Task : ObjectiveNode->Tasks)
	   {
		   if (Task)
		   {
			   Task->BeginTask();
		   }

	   }
   }

   if (UQuestSystemGraphNode_State* StateNode = Cast<UQuestSystemGraphNode_State>(QuestParam->CurrentNode))
   {
	   StateNode->BeginState();
   }
}




void UQuestComponent::Save_Implementation(const FString& SaveFileName, int SlotIndex)
{
	OnBeginSave.Broadcast(SaveFileName);
	//Create New Save
	if (UQuestSaveObject* QuestSaveObject = Cast<UQuestSaveObject>(UGameplayStatics::CreateSaveGameObject(UQuestSaveObject::StaticClass())))
	{
		//Start Saving Data
		for (const auto& It : QuestSystemMap)
		{
			FName Key = It.Key;
			UQuestSystem* QuestSystem = It.Value;
				TMap<FName, FNodeData> NodeMapData;
				for (UQuestSystemGraphNode* Node : QuestSystem->AllNodes)
				{
					TArray<FTaskData> TaskDataArray;
					if (UQuestSystemGraphNode_Objective* ObjectiveNode = Cast<UQuestSystemGraphNode_Objective>(Node))
					{
						for (UQuestTask* Task : ObjectiveNode->Tasks)
						{
							FTaskData TaskData;
							if (Task)
							{
								TaskData.CurrentAmount = Task->CurrentAmount;
								TaskData.TaskState = Task->TaskState;
							}
							TaskDataArray.Add(TaskData);
						}
					}

					TArray<FEventData> EventDataArray;
					for (UQuestEvent* Event : Node->Events)
					{
						FEventData EventData;
						if (Event)
						{
							EventData.StartLaunched = Event->StartLaunched;
							EventData.EndLaunched = Event->EndLaunched;
						}
						EventDataArray.Add(EventData);
					}

					FNodeData NodeData;
					if (Node)
					{
						NodeData.NodeID = Node->ID;
						NodeData.TaskDatas = TaskDataArray;
						NodeData.EventDatas = EventDataArray;
					}
					NodeMapData.Emplace(Node->ID, NodeData);
				}
				FQuestData QuestData;
				if (QuestSystem->CurrentNode)
				{
					QuestData.CurrentNodeID = QuestSystem->CurrentNode->ID;
				}
				QuestData.QuestState = QuestSystem->QuestState;
				QuestData.NodeMapData = NodeMapData;
				QuestData.VisitedNodeIDs = QuestSystem->VisitedNodeIDs;
				QuestSaveObject->QuestMapData.Emplace(QuestSystem->QuestID, QuestData);
			
		}

		if (UGameplayStatics::SaveGameToSlot(QuestSaveObject, SaveFileName, SlotIndex))
		{
			OnSaveComplete.Broadcast(SaveFileName);
		}
	}
}

void UQuestComponent::Load_Implementation(const FString& SaveFileName, int SlotIndex) 
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveFileName, 0))
	{
		UE_LOG(LogTemp, Warning, TEXT("Quest Component: Loading Save file failed, %s save file does not exist "), *SaveFileName);
		return;
	}
	TMap<FName, UQuestSystem*> QuestMap;
	for (TObjectIterator<UQuestSystemGraph> It; It; ++It)
	{
		for (UQuestSystem* Quest : It->QuestSystems)
		{
			QuestMap.Add(Quest->QuestID, Quest);
		}
	}

	if (UQuestSaveObject* LoadedData = Cast<UQuestSaveObject>(UGameplayStatics::LoadGameFromSlot(SaveFileName, SlotIndex)))
	{
		OnBeginLoad.Broadcast(SaveFileName);
		bQuestLoaded = true;
		for (const auto& QuestIt : LoadedData->QuestMapData)
		{
			FName QuestID = QuestIt.Key;
			FQuestData QuestData = QuestIt.Value;

			UQuestSystem* FoundedQuest = QuestMap.FindRef(QuestID);
			if (FoundedQuest != nullptr)
			{
				//Fill Quest Data Here
				FoundedQuest->QuestState = QuestData.QuestState;
				FoundedQuest->CurrentNode = FoundedQuest->NodeMap.FindRef(QuestData.CurrentNodeID);
				FoundedQuest->VisitedNodeIDs = QuestData.VisitedNodeIDs;

				for (const auto& NodeIt : QuestData.NodeMapData)
				{
					FName NodeID = NodeIt.Key;
					FNodeData NodeData = NodeIt.Value;

					UQuestSystemGraphNode* FoundedNode = FoundedQuest->NodeMap.FindRef(NodeID);
					if (FoundedNode != nullptr)
					{
						//Fill Node Data Here
						if (UQuestSystemGraphNode_Objective* ObjectiveNode = Cast<UQuestSystemGraphNode_Objective>(FoundedNode))
						{
							for (int i = 0; i < ObjectiveNode->Tasks.Num(); i++)
							{
								if (ObjectiveNode->Tasks[i] != nullptr && i < NodeData.TaskDatas.Num())
								{
									//fill task data here
									ObjectiveNode->Tasks[i]->CurrentAmount = NodeData.TaskDatas[i].CurrentAmount;
									ObjectiveNode->Tasks[i]->TaskState = NodeData.TaskDatas[i].TaskState;
								}
							}
						}

						for (int i = 0; i < FoundedNode->Events.Num(); i++)
						{
							if (FoundedNode->Events[i] != nullptr && i < NodeData.EventDatas.Num())
							{
								//fill event data here
								FoundedNode->Events[i]->StartLaunched = NodeData.EventDatas[i].StartLaunched;
								FoundedNode->Events[i]->EndLaunched = NodeData.EventDatas[i].EndLaunched;
							}
						}
					}
				}
				OnLoadComplete.Broadcast(SaveFileName);
			}
		}
	}

}

bool UQuestComponent::DeleteSave(const FString& SaveName, const int32 Slot)
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveName, 0))
	{
		return false;
	}

	return UGameplayStatics::DeleteGameInSlot(SaveName, Slot);
}

bool UQuestComponent::ShouldUpdateQuest(UQuestSystem* QuestParam)
{
	return !(QuestParam->QuestState == EQuestState::E_Complete || QuestParam->QuestState == EQuestState::E_Fail);
}




#undef LOCTEXT_NAMESPACE

void UQuestComponent::QuestObjectiveUpdated()
{
	UpdateQuest();
}

void UQuestComponent::StartNavigatingQuest(UQuestSystem* Quest)
{
	if (Quest)
	{
        UE_LOG(LogTemp, Log, TEXT("Start Navigating Quest: %s"), *Quest->QuestID.ToString());  
    }  
}

void UQuestComponent::QuestFailed(UQuestSystem* Quest)
{
	if (Quest)
	{
		UE_LOG(LogTemp, Log, TEXT("Quest failed: %s"), *Quest->QuestID.ToString());
	}
}

void UQuestComponent::QuestCompleted(UQuestSystem* Quest)
{
	if (Quest)
	{
		UE_LOG(LogTemp, Log, TEXT("Quest Completed: %s"), *Quest->QuestID.ToString());
	}
}

void UQuestComponent::QuestAdded(UQuestSystem* Quest)
{
	if (Quest)
	{
		UE_LOG(LogTemp, Log, TEXT("Quest Added: %s"), *Quest->QuestID.ToString());
	}
}

void UQuestComponent::BeginSave(FString SaveName)
{
	UE_LOG(LogTemp, Verbose, TEXT("Begun saving using save name: %s"), *SaveName);
}

void UQuestComponent::BeginLoad(FString SaveName)
{
	UE_LOG(LogTemp, Verbose, TEXT("Begun loading using save name: %s"), *SaveName);
}

void UQuestComponent::SaveComplete(FString SaveName)
{
	UE_LOG(LogTemp, Verbose, TEXT("Save complete for save name: %s"), *SaveName);
}

void UQuestComponent::LoadComplete(FString SaveName)
{
	UE_LOG(LogTemp, Verbose, TEXT("Load complete for save name: %s"), *SaveName);
}
