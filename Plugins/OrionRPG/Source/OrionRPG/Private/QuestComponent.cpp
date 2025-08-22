// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "QuestComponent.h"
#include "Quest.h"
#include "Decorator/OrionDecorator.h"
#include "Event/OrionEvent.h"
#include "UObject/UObjectIterator.h"
#include "QuestSaveObject.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode_State.h"
#include "QuestBuilderNode_Checkpoint.h"
#include "QuestBuilderNode_Root.h"
#include "QuestBuilderNode_Objective.h"
#include "QuestBuilderSetting.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Controller.h"
#include "QuestData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"


#define LOCTEXT_NAMESPACE "QuestComponent"

UQuestComponent::UQuestComponent()
{

	SetIsReplicatedByDefault(true);
	SetComponentTickEnabled(true);
	PrimaryComponentTick.bCanEverTick = true;
	bAutoNavigateQuest = true;
}


void UQuestComponent::BeginPlay()
{
	Super::BeginPlay();
	QuestMap.Empty();
	QuestCategories.Empty();
	
	//Set Dependency
	OwningController = GetOwningController();

	OnQuestUpdated.AddDynamic(this, &UQuestComponent::QuestUpdated);
	OnQuestObjectiveBegin.AddDynamic(this, &UQuestComponent::QuestObjectiveBegin);
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
}


void UQuestComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

bool UQuestComponent::BeginQuestGraph(UQuestBuilderGraph* QuestGraphAsset)
{
	if (QuestGraphAsset == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Quest Asset is null"));
		return false;
	}

	for (auto& QuestTemplate : QuestGraphAsset->QuestList)
	{
		UQuest* NewQuest = MakeQuestInstance(QuestTemplate);
		if (NewQuest)
		{
			EvaluateQuestState(NewQuest);
		}
	}

	AutoNavigateQuest();

	return true;
	
}

class UQuest* UQuestComponent::MakeQuestInstance(UQuest* QuestTemplate)
{
	if (IsValid(QuestTemplate))
	{
		//Duplicate the quest template
		UQuest* NewQuest = Cast<UQuest>(StaticDuplicateObject(QuestTemplate, this, NAME_None, RF_Transactional));
		NewQuest->Initialize(this);
		if (QuestMap.FindRef(NewQuest->ID))
		{
			//UE_LOG(LogTemp, Warning, TEXT("%s Already Registered."), *QuestTemplate->GetName());
			return nullptr;
		}

		//Add New Quest into Quest Categories
		TArray<FText> CategoryTextList;
		for (auto& QuestCategory : QuestCategories)
		{
			CategoryTextList.Add(QuestCategory.Category);
			if (QuestCategory.Category.EqualTo(NewQuest->QuestCategory))
			{
				QuestCategory.QuestList.AddUnique(NewQuest);			
			}
		}
		
        if (!CategoryTextList.ContainsByPredicate([&](const FText& CategoryText) { return CategoryText.EqualTo(NewQuest->QuestCategory); }))
        {
			FQuestCategory QuestCategory;
			QuestCategory.Category = NewQuest->QuestCategory;
			QuestCategory.QuestList.AddUnique(NewQuest);	
			QuestCategories.Add(QuestCategory);
		}
		QuestMap.Emplace(NewQuest->ID, NewQuest);
		return NewQuest;
	}

	return nullptr;
}

class UQuest* UQuestComponent::TryMakeQuestInstanceFromTag(FGameplayTag QuestTag)
{
	FName QuestID = QuestTag.GetTagName();
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> QuestGraphDataArray;
	AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UQuestBuilderGraph::StaticClass()), QuestGraphDataArray);
		
	for (const FAssetData& AssetData : QuestGraphDataArray)
	{
		if (UQuestBuilderGraph* QuestGraphAsset = Cast<UQuestBuilderGraph>(AssetData.GetAsset()))
		{
			for (auto& QuestTemplate : QuestGraphAsset->QuestList)
			{
				if (QuestID == QuestTemplate->ID ||
					QuestID == QuestTemplate->QuestTag.GetTagName())
				{
					//make quest instance from template found
					if (UQuest* QuestInstance = MakeQuestInstance(QuestTemplate))
					{
						return QuestInstance;
					}
				}
			}
		}
	}
	return nullptr;
}

void UQuestComponent::AutoNavigateQuest()
{
	if (!bAutoNavigateQuest)
		return;
	if (CurrentNavigatedQuest && CurrentNavigatedQuest->QuestState == EQuestState::E_Active)
		return;

	TArray<FName> QuestKeys;
	QuestMap.GetKeys(QuestKeys);

	for (int32 i = 0; i < QuestKeys.Num(); i++)
	{
		FName Key = QuestKeys[i];
		UQuest* Quest = QuestMap.FindRef(Key);
		if (Quest && Quest->QuestState == EQuestState::E_Active)
		{
			StartNavigateQuest(Quest);
			return;
		}
	}
}

void UQuestComponent::UpdateAllQuest()
{
	TArray<FName> QuestKeys;
	QuestMap.GetKeys(QuestKeys);

	for (int32 i = 0; i < QuestKeys.Num(); i++)
	{
		FName Key = QuestKeys[i];
		UQuest* Quest = QuestMap.FindRef(Key);
		if (Quest && Quest->QuestState == EQuestState::E_Active)
		{
			Quest->UpdateQuest();
		}
	}

}

void UQuestComponent::ActivateQuestFromTag(FGameplayTag QuestTag)
{
	FName QuestID = QuestTag.GetTagName();
	if (UQuest* Quest = QuestMap.FindRef(QuestID))
	{		
		ActivateQuest(Quest);
	}
	else
	{
		ActivateQuest(TryMakeQuestInstanceFromTag(QuestTag));
	}
}

void UQuestComponent::UnlockQuestFromTag(FGameplayTag QuestTag)
{
	FName QuestID = QuestTag.GetTagName();
	if (UQuest* Quest = QuestMap.FindRef(QuestID))
	{
		UnlockQuest(Quest);
	}
	else
	{
		UnlockQuest(TryMakeQuestInstanceFromTag(QuestTag));
	}
}

void UQuestComponent::LockQuestFromTag(FGameplayTag QuestTag)
{
	FName QuestID = QuestTag.GetTagName();
	if (UQuest* Quest = QuestMap.FindRef(QuestID))
	{
		LockQuest(Quest);
	}
	else
	{
		LockQuest(TryMakeQuestInstanceFromTag(QuestTag));
	}
}

void UQuestComponent::RestartQuestFromTag(FGameplayTag QuestTag, FGameplayTag NodeTag, const bool bNotifyQuestAdded)
{
	FName QuestID = QuestTag.GetTagName();
	FName NodeID = NodeTag.GetTagName();
	if (UQuest* FoundedQuest = QuestMap.FindRef(QuestID))
	{
		UQuestBuilderNode* FoundedNode = nullptr;
		if (!NodeTag.IsValid())
		{
			FoundedNode = FoundedQuest->NodeMap.FindRef(NodeID);
			if (FoundedNode)
			{
				//Reset all visited node that is not visited by FoundedNode
				for (int32 i = FoundedQuest->VisitedNodeIDs.Num() - 1; i >= 0; --i)
				{
					FName CachedVisitedNodeID = FoundedQuest->VisitedNodeIDs[i];
					if (UQuestBuilderNode* VisitedNode = FoundedQuest->NodeMap.FindRef(CachedVisitedNodeID))
					{
						VisitedNode->Reset();
					}
					FoundedQuest->VisitedNodeIDs.RemoveAt(i);
					if (CachedVisitedNodeID == NodeID)
						break;
				}
			}
		}
		RestartQuest(FoundedQuest, FoundedNode, bNotifyQuestAdded);
	}
	else
	{
		RestartQuest(TryMakeQuestInstanceFromTag(QuestTag), nullptr, bNotifyQuestAdded);
	}
}

void UQuestComponent::FailQuestFromTag(FGameplayTag QuestTag)
{
	FName QuestID = QuestTag.GetTagName();
	if (UQuest* Quest = QuestMap.FindRef(QuestID))
	{
		FailQuest(Quest);
	}
	else
	{
		FailQuest(TryMakeQuestInstanceFromTag(QuestTag));
	}
}

void UQuestComponent::CompleteQuestFromTag(FGameplayTag QuestTag)
{
	FName QuestID = QuestTag.GetTagName();
	if (UQuest* Quest = QuestMap.FindRef(QuestID))
	{
		CompleteQuest(Quest);
	}
	else
	{
		CompleteQuest(TryMakeQuestInstanceFromTag(QuestTag));
	}
}




void UQuestComponent::StartNavigateQuest(UQuest* InQuest, UQuestBuilderNode_Objective* InObjective)
{
	if (InQuest && InQuest->QuestState == EQuestState::E_Active)
	{
		CurrentNavigatedQuest = InQuest;
		if (InObjective)
		{
			InQuest->CurrentNavigatedObjective = InObjective;
		}
		else
		{
			InQuest->GetNavigatedObjective();
		}

		OnStartNavigateQuest.Broadcast(InQuest);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("QuestComponent : Failed to Navigate Quest. Quest not found or has Completed/Failed"));
	}
}

void UQuestComponent::ActivateQuest(UQuest* InQuest)
{
	if (InQuest && InQuest->QuestState == EQuestState::E_Locked || InQuest->QuestState == EQuestState::E_Unlocked)
	{
		InQuest->QuestState = EQuestState::E_Active;
		if (!InQuest->CurrentNodes.IsEmpty())
		{
			InQuest->BeginMultipleNodes(InQuest->CurrentNodes);
		}
		else
		{
			OnQuestAdded.Broadcast(InQuest);
			RestartQuest(InQuest);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("QuestComponent : Failed to Activate Quest, Quest not found"));
	}
	
}

void UQuestComponent::UnlockQuest(UQuest* InQuest)
{
	if (InQuest && InQuest->QuestState == EQuestState::E_Locked)
	{
		InQuest->QuestState = EQuestState::E_Unlocked;
		OnQuestUnlocked.Broadcast(InQuest);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("QuestComponent : Failed to Unlock Quest, Quest not found"));
	}
}

void UQuestComponent::LockQuest(UQuest* InQuest)
{
	if (InQuest)
	{
		InQuest->QuestState = EQuestState::E_Locked;
		InQuest->CurrentNodes.Empty();
		InQuest->VisitedNodeIDs.Empty();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("QuestComponent : Failed to Lock Quest, Quest not found"));
	}
}

void UQuestComponent::RestartQuest(UQuest* InQuest, UQuestBuilderNode* InNode, const bool bNotifyQuestAdded)
{
	if (InQuest)
	{
		InQuest->QuestState = EQuestState::E_Active;
		if(bNotifyQuestAdded)
		{
			OnQuestAdded.Broadcast(InQuest);
		}
		if (InNode)
		{
			InQuest->BeginNode(InNode);
		}
		else
		{
			//reset all Node
			for (auto& Node : InQuest->AllNodes)
			{
				Node->Reset();
			}
			InQuest->VisitedNodeIDs.Empty();
			if (InQuest->RootNodes.IsValidIndex(0))
			{
				InQuest->BeginNode(InQuest->RootNodes[0]);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("QuestComponent : Failed to Restart Quest, Quest not found"));
	}
}

void UQuestComponent::FailQuest(UQuest* InQuest)
{
	if (InQuest && InQuest->QuestState == EQuestState::E_Active)
	{
		//fail all current nodes
		for (int32 i = 0; i < InQuest->CurrentNodes.Num(); i++)
		{
			if (InQuest->CurrentNodes.IsValidIndex(i))
			{
				if (UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(InQuest->CurrentNodes[i]))
				{
					ObjectiveNode->FailObjective();
				}
			}
		}
		InQuest->QuestState = EQuestState::E_Fail;
		InQuest->CurrentNodes.Empty();

		OnQuestFailed.Broadcast(InQuest);
		OnQuestUpdated.Broadcast();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("QuestComponent : Failed to Fail Quest, Quest not found"));
	}
}

void UQuestComponent::CompleteQuest(UQuest* InQuest)
{
	if (InQuest && InQuest->QuestState == EQuestState::E_Active)
	{
		//fail all current nodes
		for (int32 i = 0; i < InQuest->CurrentNodes.Num(); i++)
		{
			if (InQuest->CurrentNodes.IsValidIndex(i))
			{
				if (UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(InQuest->CurrentNodes[i]))
				{
					ObjectiveNode->FailObjective();
				}
			}
		}
		InQuest->QuestState = EQuestState::E_Complete;
		
		InQuest->CurrentNodes.Empty();

		OnQuestCompleted.Broadcast(InQuest);
		OnQuestUpdated.Broadcast();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("QuestComponent : Failed to Complete Quest, Quest not found"));
	}
}

bool UQuestComponent::IsQuestAtState(FGameplayTag QuestTag, EQuestState QuestState)
{
	UQuest* Quest = FindQuest(QuestTag);

	if (Quest && Quest->QuestState == QuestState)
	{
		return true;
	}

	return false;
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

void UQuestComponent::StopNavigateQuest()
{
	CurrentNavigatedQuest = nullptr;
	OnStopNavigateQuest.Broadcast();
}

void UQuestComponent::DumpLog() const
{
#if !UE_BUILD_SHIPPING

	static FString Message;
	Message.Reset();

	Message += TEXT("\n==================== BEGIN Quest ====================\n");

	for (int32 i = 0; i < QuestCategories.Num(); ++i)
	{
		const FQuestCategory& QuestCategory = QuestCategories[i];

		const FString CategoryStr = QuestCategory.Category.ToString();
		Message += FString::Printf(TEXT("Category: %s\n"), *CategoryStr);
		for (auto& Quest : QuestCategory.QuestList)
		{
			const FString QuestStateStr = UEnum::GetValueAsString(Quest->QuestState);
			Message += FString::Printf(TEXT("\tQuest: %s, State: %s\n"), *Quest->ID.ToString(), *QuestStateStr);
		}
	}

	Message += TEXT("==================== END Quest ====================\n");

	UE_LOG(LogTemp, Log, TEXT(" %s"), *Message);

#endif // UE_BUILD_SHIPPING
}

UQuest* UQuestComponent::FindQuest(FGameplayTag QuestTag) const
{
	FName QuestID = QuestTag.GetTagName();
	if (UQuest* FoundedQuest = QuestMap.FindRef(QuestID))
	{
		return FoundedQuest;
	}

	UE_LOG(LogTemp, Warning, TEXT("Quest Not Found"));
	return nullptr;
}

void UQuestComponent::ResetAllQuest()
{
	TArray<FName> QuestKeys;
	QuestMap.GetKeys(QuestKeys);

	for (int32 j = QuestKeys.Num() - 1; j >= 0; --j)
	{
		FName Key = QuestKeys[j];
		UQuest* Quest = QuestMap.FindRef(Key);
		Quest->Deinitialize();
		Quest->CurrentNodes.Empty();
		QuestMap.Remove(Key);
	}
	QuestMap.Empty();
	QuestCategories.Empty();
	CurrentNavigatedQuest = nullptr;
}

void UQuestComponent::EvaluateQuestState(UQuest* QuestParam)
{
	if (QuestParam->QuestState != EQuestState::E_Locked) // only evaluate if the quest state is locked
		return;

		switch (QuestParam->DesiredQuestState)
		{
			case EQuestState::E_Active:
				ActivateQuest(QuestParam);
				return;
			case EQuestState::E_Fail:
				FailQuest(QuestParam);
				return;
			case EQuestState::E_Complete:
				CompleteQuest(QuestParam);
				return;
			case EQuestState::E_Unlocked:
				UnlockQuest(QuestParam);
				return;
		}
		
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
			QuestSaveObject->QuestAssetIDs.Add(Quest->ID);
				TMap<FName, FNodeData> NodeMapData;
				for (auto& Node : Quest->AllNodes)
				{
					FNodeData NodeData;
					if (Node)
					{
						NodeData.NodeID = Node->ID;
						NodeData.bNodeActive = Node->bNodeActive;
						if (UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(Node))
						{
							NodeData.CurrentProgress = ObjectiveNode->CurrentProgress;
							NodeData.ObjectiveState = ObjectiveNode->ObjectiveState;
						}
					}
					NodeMapData.Emplace(Node->ID, NodeData);
				}
				FQuestData QuestData;
				for (int i = 0; i < Quest->CurrentNodes.Num(); i++)
				{
					if (Quest->CurrentNodes.IsValidIndex(i))
					{
						QuestData.CurrentNodeIDs.AddUnique(Quest->CurrentNodes[i]->ID);
					}
				}
				
				QuestData.QuestState = Quest->QuestState;
				QuestData.NodeMapData = NodeMapData;
				QuestData.VisitedNodeIDs = Quest->VisitedNodeIDs;
				QuestSaveObject->QuestMapData.Emplace(Quest->ID, QuestData);
			
		}

		if (CurrentNavigatedQuest)
		{
			QuestSaveObject->NavigatedQuestID = CurrentNavigatedQuest->ID;
			if (CurrentNavigatedQuest->CurrentNavigatedObjective)
			{
				QuestSaveObject->NavigatedObjectiveID = CurrentNavigatedQuest->CurrentNavigatedObjective->ID;
			}
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
	TArray<UQuest*> LoadedQuestAssets;
	if (UQuestSaveObject* LoadedData = Cast<UQuestSaveObject>(UGameplayStatics::LoadGameFromSlot(SaveFileName, SlotIndex)))
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

		TArray<FAssetData> QuestGraphDataArray;
		AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UQuestBuilderGraph::StaticClass()), QuestGraphDataArray);

		//reset all quest before load the new quest graph asset
		ResetAllQuest();

		//Begin Questasset which their asset id has registered
		for (const FAssetData& AssetData : QuestGraphDataArray)
		{
			UQuestBuilderGraph* QuestGraphAsset = Cast<UQuestBuilderGraph>(AssetData.GetAsset());
			if (QuestGraphAsset)
			{
				for (auto& QuestTemplate : QuestGraphAsset->QuestList)
				{
					if (LoadedData->QuestAssetIDs.Contains(QuestTemplate->ID) ||
						LoadedData->QuestAssetIDs.Contains(QuestTemplate->QuestTag.GetTagName()))
					{
						//make quest instance from template found
						if (UQuest* LoadedQuestAsset = MakeQuestInstance(QuestTemplate))
						{
							LoadedQuestAssets.Add(LoadedQuestAsset);
						}
					}
				}
			}
		}

		TMap<FName, UQuest*> QuestLoadMap;
		for (auto& Quest : LoadedQuestAssets)
		{
			QuestLoadMap.Add(Quest->ID, Quest);
		}

		OnBeginLoad.Broadcast(SaveFileName);
		for (const auto& QuestIt : LoadedData->QuestMapData)
		{
			FName ID = QuestIt.Key;
			FQuestData QuestData = QuestIt.Value;

			UQuest* FoundedQuest = QuestLoadMap.FindRef(ID);
			if (FoundedQuest)
			{
				//Fill Quest Data Here
				FoundedQuest->QuestState = QuestData.QuestState;
				for (int i = 0; i < QuestData.CurrentNodeIDs.Num(); i++)
				{
					if (QuestData.CurrentNodeIDs.IsValidIndex(i))
					{
						if (UQuestBuilderNode* FoundedCurrentNode = FoundedQuest->NodeMap.FindRef(QuestData.CurrentNodeIDs[i]))
						{
							FoundedQuest->CurrentNodes.AddUnique(FoundedCurrentNode);
						}
					}
				}
				FoundedQuest->VisitedNodeIDs = QuestData.VisitedNodeIDs;

				for (const auto& NodeIt : QuestData.NodeMapData)
				{
					FName NodeID = NodeIt.Key;
					FNodeData NodeData = NodeIt.Value;

					UQuestBuilderNode* FoundedNode = FoundedQuest->NodeMap.FindRef(NodeID);
					if (FoundedNode)
					{
						//Fill Node Data Here
						FoundedNode->bNodeActive = NodeData.bNodeActive;
						if (UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(FoundedNode))
						{
							ObjectiveNode->CurrentProgress = NodeData.CurrentProgress;
							ObjectiveNode->ObjectiveState = NodeData.ObjectiveState;
						}
					}
				}
			}
		}
		for (auto& LoadedQuestAsset : LoadedQuestAssets)
		{
			EvaluateQuestState(LoadedQuestAsset);
			if (!LoadedQuestAsset->CurrentNodes.IsEmpty() && LoadedQuestAsset->QuestState == EQuestState::E_Active)
			{
				bool bLaunchEventOnLoad = true;
				LoadedQuestAsset->BeginMultipleNodes(LoadedQuestAsset->CurrentNodes, bLaunchEventOnLoad);
			}
		}

		StopNavigateQuest();
		UQuest* FoundedQuest = QuestMap.FindRef(LoadedData->NavigatedQuestID);
		UQuestBuilderNode_Objective* FoundedObjective = FoundedQuest ? Cast<UQuestBuilderNode_Objective>(FoundedQuest->NodeMap.FindRef(LoadedData->NavigatedObjectiveID)) : nullptr;

		if (FoundedQuest)
		{
			StartNavigateQuest(FoundedQuest, FoundedObjective);
		}
		else
		{
			AutoNavigateQuest();
		}
		OnLoadComplete.Broadcast(SaveFileName);
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


void UQuestComponent::QuestUpdated()
{
	UpdateAllQuest();
}

void UQuestComponent::QuestObjectiveBegin(UQuestBuilderNode_Objective* InObjective)
{
}


void UQuestComponent::QuestObjectiveUpdated(UQuest* Quest)
{
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
	AutoNavigateQuest();
	if (Quest)
	{
		UE_LOG(LogTemp, Log, TEXT("Quest failed: %s"), *Quest->ID.ToString());
	}
}

void UQuestComponent::QuestCompleted(UQuest* Quest)
{
	AutoNavigateQuest();
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


// ================================================================================================ //
// CONSOLE COMMANDS
// ================================================================================================ //
void UQuestComponent::DumpQuestLog()
{
	DumpLog();

	
}

#undef LOCTEXT_NAMESPACE
