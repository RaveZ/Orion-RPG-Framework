// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Quest.h"

#include "OrionRPG.h"
#include "QuestComponent.h"
#include "QuestBuilderSetting.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderEdge.h"
#include "QuestBuilderNode_Objective.h"
#include "QuestBuilderNode_Root.h"
#include "AssetRegistry/AssetRegistryModule.h"


#define LOCTEXT_NAMESPACE "Quest"


class FOrionRPGModule;

UQuest::UQuest()
{
	bCanQuestBeAborted = false;
	bCanRetakeQuest = false;
	QuestState = EQuestState::E_Locked;
	
}

bool UQuest::Initialize(UQuestComponent* InitializingComp)
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		//We need a valid Quest component to make a quest for 
		if (!InitializingComp)
		{
			return false;
		}
		OwningController = InitializingComp->GetOwningController();
		QuestComponent = InitializingComp;
		//ensure id matches with tag
		if (QuestTag.IsValid())
		{
			ID = QuestTag.GetTagName();
		}
		NodeMap.Reset();
		for (auto& Node : AllNodes)
		{
			if (Node)
			{
				Node->Quest = this;
				Node->OwningController = InitializingComp->GetOwningController();
				Node->QuestComponent = InitializingComp;
				
				if (Node->NodeTag.IsValid())
				{
					Node->ID = Node->NodeTag.GetTagName();
				}
				NodeMap.Emplace(Node->ID, Node);
			}
		}
		return true;
	}
	return false;
}

void UQuest::Deinitialize()
{
	VisitedNodeIDs.Empty();
	CurrentNodes.Empty();
	CurrentNavigatedObjective = nullptr;


	// Clean up references safely: do not force destroy UObjects that may still be referenced elsewhere.
	for (auto& Node : AllNodes)
	{
		if (IsValid(Node))
		{
			Node->Quest = nullptr;
			Node->QuestComponent = nullptr;
			Node->OwningController = nullptr;
			Node->ParentNodes.Empty();
			Node->ChildrenNodes.Empty();
			Node->Edges.Empty();
			// reset node-specific runtime state (if available)
			Node->Reset();
		}
	}

	AllNodes.Empty();
	RootNodes.Empty();
	NodeMap.Empty();
}

UWorld* UQuest::GetWorld() const
{
	if (QuestComponent)
	{
		return QuestComponent->GetWorld();
	}

	return nullptr;
}

void UQuest::UpdateQuest()
{
	if (QuestState != EQuestState::E_Active)
		return;
	
	//Cache Current nodes, because some node may be removed from current nodes after branching, 
	TArray<UQuestBuilderNode*> CachedCurrentNodes = CurrentNodes;
	for (int i = 0; i < CachedCurrentNodes.Num(); i++)
	{
		if (CachedCurrentNodes.IsValidIndex(i) && CurrentNodes.Contains(CachedCurrentNodes[i]))
		{
			CachedCurrentNodes[i]->EvaluateNextNode();
		}
	}

}

TArray<UQuestBuilderNode*> UQuest::GetVisitedNodes()
{
	SortVisitedNodes();

	TArray<UQuestBuilderNode*> ReturnNodes;
	for (auto& VisitedNodeID : VisitedNodeIDs)
	{
		UQuestBuilderNode* QuestNode = NodeMap.FindRef(VisitedNodeID);
		ReturnNodes.AddUnique(QuestNode);
	}
	return ReturnNodes;
}

TArray<UQuestBuilderNode_Objective*> UQuest::GetCurrentObjectives()
{
	TArray<UQuestBuilderNode_Objective*> ReturnNodes;
	for (auto& CurrNode : CurrentNodes)
	{
		UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(CurrNode);
		if (ObjectiveNode && ObjectiveNode->IsObjectiveActive())
		{
			ReturnNodes.AddUnique(ObjectiveNode);
		}
	}
	return ReturnNodes;
}

void UQuest::SortVisitedNodes()
{
	//Sort Visited nodes in this order:
	// Ongoing Objectives > Completed Objectives > Failed Objective
	TArray<FName> CachedVisitedNodeIDs;
	for (auto& VisitedNodeID : VisitedNodeIDs)
	{
		UQuestBuilderNode* QuestNode = NodeMap.FindRef(VisitedNodeID);
		if (QuestNode && !QuestNode->bHidden)
		{
			UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(QuestNode);
			if (ObjectiveNode && ObjectiveNode->IsObjectiveFailed())
			{
				CachedVisitedNodeIDs.Add(QuestNode->ID);
			}
		}
	}
	for (auto& VisitedNodeID : VisitedNodeIDs)
	{
		UQuestBuilderNode* QuestNode = NodeMap.FindRef(VisitedNodeID);
		if (QuestNode && !QuestNode->bHidden)
		{
			UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(QuestNode);
			if (ObjectiveNode && ObjectiveNode->IsObjectiveCompleted())
			{
				CachedVisitedNodeIDs.Add(QuestNode->ID);
			}
		}
	}
	
	for (auto& VisitedNodeID : VisitedNodeIDs)
	{
		UQuestBuilderNode* QuestNode = NodeMap.FindRef(VisitedNodeID);
		if (QuestNode && !QuestNode->bHidden)
		{
			if (UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(QuestNode))
			{
				if (ObjectiveNode->IsObjectiveActive())
				{
					CachedVisitedNodeIDs.Add(QuestNode->ID);
				}
			}
			else
			{
				CachedVisitedNodeIDs.Add(QuestNode->ID);
			}
		}
	}
	VisitedNodeIDs = CachedVisitedNodeIDs;
}

UQuestBuilderNode_Objective* UQuest::GetNavigatedObjective()
{
	if (!CurrentNavigatedObjective)
	{
        for (int32 i = CurrentNodes.Num() - 1; i >= 0; --i)
        {
			UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(CurrentNodes[i]);
			if (ObjectiveNode && ObjectiveNode->IsObjectiveActive())
			{
				CurrentNavigatedObjective = ObjectiveNode;
				break;
			}
        }
	}
	return CurrentNavigatedObjective;
}

void UQuest::BeginNode(UQuestBuilderNode* InNode, bool bLaunchEventOnLoad)
{
	if (InNode == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("BeginNode: Node is null for Quest ID: %s"), *ID.ToString());
		return;
	}

	if (InNode)
	{
		CurrentNodes.AddUnique(InNode);
	}
	
	
	if (UQuestBuilderNode_Root* RootNode = Cast<UQuestBuilderNode_Root>(InNode))
	{
		RootNode->Initialize();
		return;
	}
	
	//add visited node
	VisitedNodeIDs.AddUnique(InNode->ID);
	
	//InitializeNode
	InNode->Initialize(bLaunchEventOnLoad);
}

void UQuest::BeginMultipleNodes(TArray<UQuestBuilderNode*> InNodes, bool bLaunchEventOnLoad)
{
	for (auto& node : InNodes)
	{
		BeginNode(node, bLaunchEventOnLoad);
	}
}

int UQuest::GetLevelNum() const
{
	int Level = 0;
	TArray<UQuestBuilderNode*> CurrLevelNodes = RootNodes;
	TArray<UQuestBuilderNode*> NextLevelNodes;

	while (CurrLevelNodes.Num() != 0)
	{
		for (int i = 0; i < CurrLevelNodes.Num(); ++i)
		{
			UQuestBuilderNode* Node = CurrLevelNodes[i];
			check(Node != nullptr);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
			{
				NextLevelNodes.Add(Node->ChildrenNodes[j]);
			}
		}

		CurrLevelNodes = NextLevelNodes;
		NextLevelNodes.Reset();
		++Level;
	}

	return Level;
}

void UQuest::GetNodesByLevel(int Level, TArray<UQuestBuilderNode*>& Nodes)
{
	int CurrLEvel = 0;
	TArray<UQuestBuilderNode*> NextLevelNodes;

	Nodes = RootNodes;

	while (Nodes.Num() != 0)
	{
		if (CurrLEvel == Level)
			break;

		for (int i = 0; i < Nodes.Num(); ++i)
		{
			UQuestBuilderNode* Node = Nodes[i];
			check(Node != nullptr);

			for (int j = 0; j < Node->ChildrenNodes.Num(); ++j)
			{
				NextLevelNodes.Add(Node->ChildrenNodes[j]);
			}
		}

		Nodes = NextLevelNodes;
		NextLevelNodes.Reset();
		++CurrLEvel;
	}
}

APlayerController* UQuest::GetOwningController()
{
	return OwningController;
}

APawn* UQuest::GetOwningPawn()
{
	if (QuestComponent)
	{
		return QuestComponent->GetOwningPawn();
	}
	return nullptr;
}

UQuestComponent* UQuest::GetQuestComponent()
{
	return QuestComponent;
}

UQuestBuilderGraph* UQuest::GetOwningQuestGraph() const
{
	if (!QuestGraph)
		return nullptr;

	return QuestGraph;
}

void UQuest::ClearGraph()
{
	for (int i = 0; i < AllNodes.Num(); ++i)
	{
		UQuestBuilderNode* Node = AllNodes[i];
		if (Node)
		{
			Node->ParentNodes.Empty();
			Node->ChildrenNodes.Empty();
			Node->Edges.Empty();
		}
	}

	VisitedNodeIDs.Empty();
	AllNodes.Empty();
	RootNodes.Empty();
	NodeMap.Empty();
}

void UQuest::EvaluateUniqueID()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> QuestGraphDataArray;
	AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UQuestBuilderGraph::StaticClass()), QuestGraphDataArray);

	TArray<UQuest*> Quests;
	TArray<FName> QuestIDs;

	for (const FAssetData& AssetData : QuestGraphDataArray)
	{
		UQuestBuilderGraph* QuestGraphAsset = Cast<UQuestBuilderGraph>(AssetData.GetAsset());
		if (QuestGraphAsset)
		{
			for (auto& Quest : QuestGraphAsset->QuestList)
			{
				Quests.Add(Quest);
			}
		}
	}

	for (auto& Quest : Quests)
	{
		if (Quest != this)
		{
			QuestIDs.Add(Quest->ID);
		}
	}

	int32 Suffix = 1;
	FName NewID = ID;

	if (!QuestIDs.Contains(NewID))
	{
		return;
	}

	// Check if the new ID already exists in the array
	while (QuestIDs.Contains(NewID))
	{
		// If it does, add a numeric suffix and try again
		NewID = FName(*FString::Printf(TEXT("%s%d"), *ID.ToString(), Suffix));
		Suffix++;
	}

	ID = NewID;

}


void UQuest::MakeQuestSettingShareable(FString ShareName, bool bInitialize)
{
	SharedCategoryIdx = INDEX_NONE;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> QuestgraphDataArray;
	AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UQuestBuilderGraph::StaticClass()), QuestgraphDataArray);

	TArray<int32> Remap;

	if (bInitialize)
	{
		FOrionRPGModule& OrionRPGModule = FModuleManager::GetModuleChecked<FOrionRPGModule>(TEXT("OrionRPG"));
		for (auto& QuestPreset : OrionRPGModule.QuestPresets)
		{
			if (QuestPreset == this || QuestPreset->SharedCategoryIdx != INDEX_NONE)
			{
				QuestPreset->SharedCategoryIdx = Remap.AddUnique(QuestPreset->SharedCategoryIdx) + 1; // Remaps existing index to lowest index available
			}
		}
	}
	else
	{
		for (const FAssetData& AssetData : QuestgraphDataArray)
		{
			UQuestBuilderGraph* QuestGraphAsset = Cast<UQuestBuilderGraph>(AssetData.GetAsset());
			if (QuestGraphAsset)
			{
				for (auto& OtherQuest : QuestGraphAsset->QuestList)
				{
					if (OtherQuest->SharedCategoryIdx != INDEX_NONE || OtherQuest == this)
					{
						OtherQuest->SharedCategoryIdx = Remap.AddUnique(OtherQuest->SharedCategoryIdx) + 1; // Remaps existing index to lowest index available
					}
				}
			}
		}
	}




	bSharedCategory = true;
	SharedCategoryName = ShareName;
	SharedCategoryGuid = FGuid::NewGuid();
}


void UQuest::UnshareQuestSetting()
{
	bSharedCategory = false;
	SharedCategoryIdx = INDEX_NONE;
	SharedCategoryName.Empty();
	SharedCategoryGuid.Invalidate();
}

void UQuest::UseSharedQuestSetting(const UQuest* Quest)
{
	if (Quest == this || Quest == nullptr)
	{
		return;
	}


	Modify();

	bSharedCategory = Quest->bSharedCategory;
	SharedCategoryName = Quest->SharedCategoryName;
	SharedCategoryGuid = Quest->SharedCategoryGuid;
	CopyQuestSettings(Quest);
}

void UQuest::CopyQuestSettings(const UQuest* SrcQuest)
{
	QuestCategory = SrcQuest->QuestCategory;
	bCanQuestBeAborted = SrcQuest->bCanQuestBeAborted;
	bCanRetakeQuest = SrcQuest->bCanRetakeQuest;
	SharedCategoryIdx = SrcQuest->SharedCategoryIdx;
	SharedCategoryName = SrcQuest->SharedCategoryName;
	SharedCategoryGuid = SrcQuest->SharedCategoryGuid;
}

void UQuest::PropagateQuestSettings()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> QuestgraphDataArray;
	AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UQuestBuilderGraph::StaticClass()), QuestgraphDataArray);

	for (const FAssetData& AssetData : QuestgraphDataArray)
	{
		UQuestBuilderGraph* QuestGraphAsset = Cast<UQuestBuilderGraph>(AssetData.GetAsset());
		if (QuestGraphAsset)
		{
			for (int32 idx = 0; idx < QuestGraphAsset->QuestList.Num(); idx++)
			{
				if (UQuest* Quest = QuestGraphAsset->QuestList[idx])
				{
					if (Quest->SharedCategoryIdx != INDEX_NONE && Quest->SharedCategoryGuid == SharedCategoryGuid)
					{
						Quest->Modify();
						Quest->CopyQuestSettings(this);
					}
				}
			}
		}
	}
	FOrionRPGModule& OrionRPGModule = FModuleManager::GetModuleChecked<FOrionRPGModule>(TEXT("OrionRPG"));
	for (auto& QuestPreset : OrionRPGModule.QuestPresets)
	{
		if (QuestPreset && QuestPreset->SharedCategoryIdx != INDEX_NONE && QuestPreset->SharedCategoryGuid == SharedCategoryGuid)
		{
			QuestPreset->CopyQuestSettings(this);
		}
	}
}


void IQuestSharedDataHelper::MakeSureGuidExists(UQuest* Quest)
{
	if (!Quest || !Quest->QuestGraph)
		return;

	UQuestBuilderGraph* CurrentGraph = Quest->QuestGraph;
	for (int32 idx = 0; idx < CurrentGraph->QuestList.Num(); idx++)
	{
		if (UQuest* OtherQuest = CurrentGraph->QuestList[idx])
		{
			if (OtherQuest != Quest &&
				CheckIfQuestShouldShareData(Quest, OtherQuest))
			{
				AccessShareDataName(Quest) = AccessShareDataName(OtherQuest);
			}
		}
	}

	if (!AccessShareDataGuid(Quest).IsValid())
	{
		AccessShareDataGuid(Quest) = FGuid::NewGuid();
	}
}

bool FQuestSharedSettingHelper::CheckIfQuestShouldShareData(const UQuest* QuestA, const UQuest* QuestB)
{
	return QuestA->bSharedCategory && QuestB->bSharedCategory && QuestA->SharedCategoryGuid == QuestB->SharedCategoryGuid;
}

bool FQuestSharedSettingHelper::CheckIfHasDataToShare(const UQuest* Quest)
{
	return Quest->SharedCategoryIdx != INDEX_NONE;
}

void FQuestSharedSettingHelper::ShareData(UQuest* QuestWhoWantsToShare, const UQuest* ShareFrom)
{
	QuestWhoWantsToShare->UseSharedQuestSetting(ShareFrom);
}

FString& FQuestSharedSettingHelper::AccessShareDataName(UQuest* Quest)
{
	return Quest->SharedCategoryName;
}

FGuid& FQuestSharedSettingHelper::AccessShareDataGuid(UQuest* Quest)
{
	return Quest->SharedCategoryGuid;
}

void UQuest::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
}

#if WITH_EDITOR

void UQuest::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UQuest, QuestCategory) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UQuest, bCanRetakeQuest) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UQuest, bCanQuestBeAborted))
	{
		PropagateQuestSettings();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty)
	{
		//If we changed the ID, make sure it doesn't conflict with any other IDs in the quest
		if (PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UQuest, ID))
		{
			EvaluateUniqueID();
		}
	}
}

void UQuest::PostLoad()
{
	Super::PostLoad();

	// make sure we have guid for shared quest setting 
	if (bSharedCategory && !SharedCategoryGuid.IsValid())
	{
		FQuestSharedSettingHelper().MakeSureGuidExists(this);
	}
}



#endif

#undef LOCTEXT_NAMESPACE
