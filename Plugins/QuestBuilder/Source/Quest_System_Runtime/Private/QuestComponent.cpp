// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "QuestComponent.h"
#include "Quest.h"
#include "Decorator/OrionDecorator.h"
#include "Event/OrionEvent.h"
#include "Task/QuestTask.h"
#include "UObject/UObjectIterator.h"
#include "QuestSaveObject.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode_State.h"
#include "QuestBuilderNode_Root.h"
#include "QuestBuilderNode_Objective.h"
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
	QuestMap.Empty();
	NodeMap.Empty();
	bQuestLoaded = false;
	//Set Dependency
	OwningController = GetOwningController();

	OnQuestObjectiveUpdated.AddDynamic(this, &UQuestComponent::QuestObjectiveUpdated);
	OnStartNavigateQuest.AddDynamic(this, &UQuestComponent::StartNavigatingQuest);
	OnQuestFailed.AddDynamic(this, &UQuestComponent::QuestFailed);
	OnQuestCompleted.AddDynamic(this, &UQuestComponent::QuestCompleted);
	OnQuestAdded.AddDynamic(this, &UQuestComponent::QuestAdded);
	OnQuestUnlocked.AddDynamic(this, &UQuestComponent::QuestUnlocked);
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

bool UQuestComponent::BeginQuest(UQuestBuilderGraph* QuestAsset)
{
	if (QuestAsset == nullptr)
	{
		return false;
	}

	bool bSuccess = false;

	UQuestBuilderGraph* NewQuestGraph = MakeQuestGraphInstance(QuestAsset);
	if (NewQuestGraph)
	{
		CacheQuestGraphDependencies(NewQuestGraph);
		EvaluateQuestSaveLoadState(NewQuestGraph);

		for (UQuest* Quest : NewQuestGraph->QuestList)
		{
			EvaluateQuestCondition(Quest);
		}
		bSuccess = true;
	}
	
	return bSuccess;
}

bool UQuestComponent::BeginMultipleQuest(TArray<UQuestBuilderGraph*> QuestAssets)
{
	if (QuestAssets.IsEmpty())
	{
		return false;
	}


	for (UQuestBuilderGraph* QuestAsset : QuestAssets)
	{
		BeginQuest(QuestAsset);
	}
	bool bSuccess = true;
	return bSuccess;
}

void UQuestComponent::CacheQuestGraphDependencies(UQuestBuilderGraph* QuestGraph)
{
	if (QuestGraph)
	{
		QuestGraph->OwningController = OwningController;
		QuestGraph->QuestComponent = this;
		QuestGraph->OwningPawn = GetOwningPawn();
		

		//Add QuestGraph to array
		QuestGraphs.Add(QuestGraph);

		for (auto& Quest : QuestGraph->QuestList)
		{
			QuestMap.Emplace(Quest->ID, Quest);
			for (auto& Node : Quest->AllNodes)
			{
				NodeMap.Emplace(Node->ID, Node);
			}
		}
	}
}

UQuestBuilderGraph* UQuestComponent::GetQuestGraphInstance(UQuestBuilderGraph* QuestGraph) const
{
	if (QuestGraph == nullptr)
		return nullptr;
	
	for (auto& Inst : QuestGraphs)
	{
		if (Inst && Inst->GetClass()->IsChildOf(QuestGraph->GetClass()))
		{
			return Inst;
		}
	}
	return nullptr;
}

class UQuestBuilderGraph* UQuestComponent::MakeQuestGraphInstance(UQuestBuilderGraph* QuestGraphTemplate)
{
	if (IsValid(QuestGraphTemplate))
	{
		//Duplicate the quest graph
		UQuestBuilderGraph* NewQuestGraph = Cast<UQuestBuilderGraph>(StaticDuplicateObject(QuestGraphTemplate, this, NAME_None, RF_Transactional));
		NewQuestGraph->SetFlags(RF_Transient | RF_DuplicateTransient);
		return NewQuestGraph;
		

	}
	return nullptr;
}

void UQuestComponent::EvaluateQuestSaveLoadState(UQuestBuilderGraph* QuestGraph)
{
	for (auto& Quest : QuestGraph->QuestList)
	{
		//check if quest has been loaded
		if (!bQuestLoaded)
		{
			LockQuest(Quest);
		}
	}
}

void UQuestComponent::ActivateQuest(FName ID)
{
	if (UQuest* Quest = QuestMap.FindRef(ID))
	{
		Quest->QuestState = EQuestState::E_Active;
		if (Quest->CurrentNode != nullptr)
		{
			BeginNode(Quest);
		}
		else if (Quest->CurrentNode == nullptr)
		{
			OnQuestAdded.Broadcast(Quest);
			RestartQuest(Quest->ID);
		}
	}
}

void UQuestComponent::UnlockQuest(FName ID)
{
	if (UQuest* Quest = QuestMap.FindRef(ID))
	{
		Quest->QuestState = EQuestState::E_Unlocked;
		OnQuestUnlocked.Broadcast(Quest);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("QuestComponent : Quest %s not found or null"), *ID.ToString());
	}
}

void UQuestComponent::LockQuest(UQuest* InQuest, FName ID)
{
	UQuest* Quest = nullptr;
	if (InQuest)
		Quest = InQuest;
	else
		Quest = QuestMap.FindRef(ID);

	if (Quest)
	{
		Quest->QuestState = EQuestState::E_Locked;
		Quest->CurrentNode = nullptr;
		Quest->VisitedNodeIDs.Empty();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("QuestComponent : Quest %s not found or null"), *ID.ToString());
	}
}

void UQuestComponent::RestartQuest(FName ID, FName NodeID, bool bStartFromNodeID)
{
	if (UQuest* Quest = QuestMap.FindRef(ID))
	{
		Quest->QuestState = EQuestState::E_Active;


		bool bNodeFound = false;
		if (NodeID.IsNone())
			bStartFromNodeID = false;

		if (bStartFromNodeID)
		{	
			if (UQuestBuilderNode* FoundedNode = Quest->NodeMap.FindRef(NodeID))
			{
				bNodeFound = true;
				//reset visited node children where the node started from
				bool bCanDelete = false;
				for (int i = 0; i < Quest->VisitedNodeIDs.Num(); i++)
				{
					if (Quest->VisitedNodeIDs[i] == NodeID)
					{
						bCanDelete = true;
					}
					if (bCanDelete)
					{
						Quest->VisitedNodeIDs.RemoveAt(i);
						i--;//ensure the next element not skipped
					}
				}

				//reset task data from the founded node
				TArray<UQuestBuilderNode*> NodeStack;
				NodeStack.Add(FoundedNode);
				while (NodeStack.Num() > 0)
				{
					UQuestBuilderNode* CurrentNodeStack = NodeStack.Pop();
					if (UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(CurrentNodeStack))
					{
						for (auto& Task : ObjectiveNode->Tasks)
						{
							if (Task)
							{
								Task->TaskState = ETaskState::E_Active;
								Task->CurrentAmount = 0;
							}
						}
					}
					for (auto& Event : CurrentNodeStack->Events)
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

				Quest->CurrentNode = FoundedNode;
			}
		}
		

		if(!bNodeFound)
		{
			//reset all task data
			for (auto& Node : Quest->AllNodes)
			{
				if (UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(Node))
				{
					for (auto& Task : ObjectiveNode->Tasks)
					{
						if (Task)
						{
							Task->TaskState = ETaskState::E_Active;
							Task->CurrentAmount = 0;
						}
						
					}
				}
				for (auto& Event : Node->Events)
				{
					if (Event)
					{
						Event->StartLaunched = false;
						Event->EndLaunched = false;
					}
				}
			}
			Quest->VisitedNodeIDs.Empty();
			Quest->CurrentNode = Quest->RootNodes[0];
		}
		

		BeginNode(Quest);
	}
}

void UQuestComponent::FailQuest(FName ID)
{
	if (UQuest* Quest = QuestMap.FindRef(ID))
	{
		if (Quest->QuestState != EQuestState::E_Fail)
		{
			Quest->QuestState = EQuestState::E_Fail;
			OnQuestFailed.Broadcast(Quest);
		}
		Quest->CurrentNode = nullptr;

	}
}

void UQuestComponent::CompleteQuest(FName ID)
{
	if (UQuest* Quest = QuestMap.FindRef(ID))
	{
		if (Quest->QuestState != EQuestState::E_Complete)
		{
			Quest->QuestState = EQuestState::E_Complete;
			OnQuestCompleted.Broadcast(Quest);
		}
		Quest->CurrentNode = nullptr;

	}
}

void UQuestComponent::StartNavigateQuest(UQuest* InQuest)
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

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	APawn* OwningPawn = Cast<APawn>(GetOwner());

	if (OwningPawn)
	{
		return OwningPawn;
	}

	if (!OwningPawn && PC)
	{
		return PC->GetPawn();
	}

	return nullptr;
}

APlayerController* UQuestComponent::GetOwningController() const
{
	//We cache this on beginplay as to not re-find it every time 
	if (OwningController)
	{
		return OwningController;
	}

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	APawn* OwningPawn = Cast<APawn>(GetOwner());

	if (PC)
	{
		return PC;
	}

	if (!PC && OwningPawn)
	{
		return Cast<APlayerController>(OwningPawn->GetController());
	}

	return nullptr;
}

UQuest* UQuestComponent::FindQuestFromNodeID(FName NodeIDParam) const
{
	UQuest* FoundedQuest = nullptr;
	for (const auto& It : QuestMap)
	{
		FName Key = It.Key;
		UQuest* Quest = It.Value;

		if (UQuestBuilderNode* QuestNode = Quest->NodeMap.FindRef(NodeIDParam))
		{
			FoundedQuest = Quest;
		}
	}

	return FoundedQuest;

}

UQuest* UQuestComponent::FindQuest(FName QuestIDParam) const
{
	if (UQuest* FoundedQuest = QuestMap.FindRef(QuestIDParam))
	{
		return FoundedQuest;
	}

	UE_LOG(LogTemp, Warning, TEXT("Quest Not Found"));
	return nullptr;
}

bool UQuestComponent::IsQuestActiveFromNodeID(FName NodeID) const
{
	if (UQuest* FoundedQuest = FindQuestFromNodeID(NodeID))
	{
		if (FoundedQuest->QuestState == EQuestState::E_Active)
		{
			return true;
		}
	}

	return false;
}

bool UQuestComponent::IsQuestActiveFromQuestID(FName ID) const
{
	if (UQuest* FoundedQuest = QuestMap.FindRef(ID))
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
	for (const auto& It : QuestMap)
	{
		FName Key = It.Key;
		UQuest* Quest = It.Value;

		EvaluateQuestCondition(Quest);

		if(Quest->QuestState == EQuestState::E_Active)
		{
			if (Quest->CurrentNode != nullptr)
			{
				if (UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(Quest->CurrentNode))
				{
					if (ObjectiveNode->IsAllTaskCompleted())
					{
						//evaluate next node condition
						EvaluateNextNodeCondition(Quest);
					}
				}
			}
			else if (Quest->CurrentNode == nullptr)
			{
				RestartQuest(Quest->ID);
			}
		}
		
		
	}
}

void UQuestComponent::EvaluateNextNodeCondition(UQuest* QuestParam)
{
	UQuestBuilderNode* NextNode = nullptr;
	for (auto& Node : QuestParam->CurrentNode->ChildrenNodes)
	{
		if (NextNode == nullptr && Node->Conditions.IsEmpty())
		{
			NextNode = Node;
		}
		else if (!Node->Conditions.IsEmpty())
		{
			int SumConditionMet = 0;
			for (auto& Condition : Node->Conditions)
			{
				bool bConditionMet = Condition && Condition->InvertCondition ? !Condition->PerformConditionCheck(OwningController, GetOwningPawn()) : Condition->PerformConditionCheck(OwningController, GetOwningPawn());
				if (bConditionMet)
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
		QuestParam->CurrentNode->BeginEvents(EEventRunType::E_End);
		NextNode->BeginEvents(EEventRunType::E_Start);

		//Go to next node if exist or condition met
		GoToNextNode(QuestParam, NextNode);
	}
	
}

void UQuestComponent::EvaluateQuestCondition(UQuest* QuestParam)
{
	if (QuestParam->QuestState != EQuestState::E_Locked)
		return;

	int SumConditionMet = 0;
	for (auto& Condition : QuestParam->Conditions)
	{
		bool bConditionMet = Condition && Condition->InvertCondition ? !Condition->PerformConditionCheck(OwningController, GetOwningPawn()) : Condition->PerformConditionCheck(OwningController, GetOwningPawn());
		if (bConditionMet)
		{
			SumConditionMet++;
		}
	}
	EvaluateQuestState(QuestParam, SumConditionMet);
	
}

void UQuestComponent::EvaluateQuestState(UQuest* QuestParam, int SumConditionMet)
{
	if (QuestParam->QuestState == EQuestState::E_Complete || QuestParam->QuestState == EQuestState::E_Fail)
		return;

	if (SumConditionMet >= QuestParam->Conditions.Num())
	{
		switch (QuestParam->DesiredQuestState)
		{
			case EQuestState::E_Active:
				ActivateQuest(QuestParam->ID);
				return;
			case EQuestState::E_Fail:
				FailQuest(QuestParam->ID);
				return;
			case EQuestState::E_Complete:
				CompleteQuest(QuestParam->ID);
				return;
			case EQuestState::E_Unlocked:
				UnlockQuest(QuestParam->ID);
				return;
		}
		
	}
}

void UQuestComponent::GoToNextNode(UQuest* QuestParam, UQuestBuilderNode* NodeParam)
{
	QuestParam->CurrentNode = NodeParam;
	BeginNode(QuestParam);
}

void UQuestComponent::BeginNode(UQuest* QuestParam)  
{  
   if (QuestParam->CurrentNode == nullptr)  
   {  
       UE_LOG(LogTemp, Warning, TEXT("BeginNode: CurrentNode is null for Quest ID: %s"), *QuestParam->ID.ToString());  
       return;  
   }  

   //add visited node
   QuestParam->VisitedNodeIDs.AddUnique(QuestParam->CurrentNode->ID);

   if (UQuestBuilderNode_Root* RootNode = Cast<UQuestBuilderNode_Root>(QuestParam->CurrentNode))
   {
	   EvaluateNextNodeCondition(QuestParam);
	   return;
   }

   //InitializeNode
   QuestParam->CurrentNode->Initialize();
}




void UQuestComponent::Save_Implementation(const FString& SaveFileName, int SlotIndex)
{
	OnBeginSave.Broadcast(SaveFileName);
	//Create New Save
	if (UQuestSaveObject* QuestSaveObject = Cast<UQuestSaveObject>(UGameplayStatics::CreateSaveGameObject(UQuestSaveObject::StaticClass())))
	{
		//Start Saving Data
		for (const auto& It : QuestMap)
		{
			FName Key = It.Key;
			UQuest* Quest = It.Value;
				TMap<FName, FNodeData> NodeMapData;
				for (auto& Node : Quest->AllNodes)
				{
					TArray<FTaskData> TaskDataArray;
					if (UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(Node))
					{
						for (auto& Task : ObjectiveNode->Tasks)
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
					for (auto& Event : Node->Events)
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
				if (Quest->CurrentNode)
				{
					QuestData.CurrentNodeID = Quest->CurrentNode->ID;
				}
				QuestData.QuestState = Quest->QuestState;
				QuestData.NodeMapData = NodeMapData;
				QuestData.VisitedNodeIDs = Quest->VisitedNodeIDs;
				QuestSaveObject->QuestMapData.Emplace(Quest->ID, QuestData);
			
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
	TMap<FName, UQuest*> QuestLoadMap;
	for (TObjectIterator<UQuestBuilderGraph> It; It; ++It)
	{
		for (auto& Quest : It->QuestList)
		{
			QuestLoadMap.Add(Quest->ID, Quest);
		}
	}

	if (UQuestSaveObject* LoadedData = Cast<UQuestSaveObject>(UGameplayStatics::LoadGameFromSlot(SaveFileName, SlotIndex)))
	{
		OnBeginLoad.Broadcast(SaveFileName);
		bQuestLoaded = true;
		for (const auto& QuestIt : LoadedData->QuestMapData)
		{
			FName ID = QuestIt.Key;
			FQuestData QuestData = QuestIt.Value;

			UQuest* FoundedQuest = QuestLoadMap.FindRef(ID);
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

					UQuestBuilderNode* FoundedNode = FoundedQuest->NodeMap.FindRef(NodeID);
					if (FoundedNode != nullptr)
					{
						//Fill Node Data Here
						if (UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(FoundedNode))
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




#undef LOCTEXT_NAMESPACE

void UQuestComponent::QuestObjectiveUpdated()
{
	UpdateQuest();
}

void UQuestComponent::StartNavigatingQuest(UQuest* Quest)
{
	if (Quest)
	{
        UE_LOG(LogTemp, Log, TEXT("Start Navigating Quest: %s"), *Quest->ID.ToString());  
    }  
}

void UQuestComponent::QuestFailed(UQuest* Quest)
{
	if (Quest)
	{
		UE_LOG(LogTemp, Log, TEXT("Quest failed: %s"), *Quest->ID.ToString());
	}
}

void UQuestComponent::QuestCompleted(UQuest* Quest)
{
	if (Quest)
	{
		UE_LOG(LogTemp, Log, TEXT("Quest Completed: %s"), *Quest->ID.ToString());
	}
}

void UQuestComponent::QuestAdded(UQuest* Quest)
{
	if (Quest)
	{
		UE_LOG(LogTemp, Log, TEXT("Quest Added: %s"), *Quest->ID.ToString());
	}
}

void UQuestComponent::QuestUnlocked(UQuest* Quest)
{
	if (Quest)
	{
		UE_LOG(LogTemp, Log, TEXT("Quest Unlocked: %s"), *Quest->ID.ToString());
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
