// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "Quest.h"
#include "QuestComponent.h"
#include "QuestBuilderSetting.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderEdge.h"
#include "QuestBuilderNode_Objective.h"
#include "QuestBuilderNode_Root.h"
#include "AssetRegistry/AssetRegistryModule.h"


#define LOCTEXT_NAMESPACE "Quest"



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
	}
	return false;
}

void UQuest::Deinitialize()
{
	VisitedNodeIDs.Empty();
	CurrentNodes.Empty();
	
	for (auto& Node : AllNodes)
	{
		Node->Deinitialize();
	}
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

void UQuest::SortVisitedNodes()
{
	//Sort Visited nodes in this order:
	// Ongoing Objectives > Completed Objectives > Failed Objective
	TArray<FName> CachedVisitedNodeIDs;
	for (auto& VisitedNodeID : VisitedNodeIDs)
	{
		UQuestBuilderNode* QuestNode = NodeMap.FindRef(VisitedNodeID);
		if (QuestNode)
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
		if (QuestNode)
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
		if (QuestNode)
		{
			UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(QuestNode);
			if (ObjectiveNode && ObjectiveNode->IsObjectiveActive())
			{
				CachedVisitedNodeIDs.Add(QuestNode->ID);
			}
		}
	}
	VisitedNodeIDs = CachedVisitedNodeIDs;
}

void UQuest::BeginNode(UQuestBuilderNode* InNode, bool bLaunchEventOnLoad)
{
	if (InNode)
	{
		CurrentNodes.AddUnique(InNode);
		if (UQuestBuilderNode_Objective* ObjectiveNode = Cast<UQuestBuilderNode_Objective>(InNode))
		{
			OnQuestObjectiveAdded.Broadcast(ObjectiveNode);
		}
	}
	if (InNode == nullptr)  
	{  
		UE_LOG(LogTemp, Warning, TEXT("BeginNode: Node is null for Quest ID: %s"), *ID.ToString());  
		return;  
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

AController* UQuest::GetOwningController()
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

TArray<FName> UQuest::GetQuestCategoryList()
{
	return GetDefault<UQuestBuilderSetting>()->QuestCategoryList;
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

#if WITH_EDITOR
void UQuest::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
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
#endif

#undef LOCTEXT_NAMESPACE