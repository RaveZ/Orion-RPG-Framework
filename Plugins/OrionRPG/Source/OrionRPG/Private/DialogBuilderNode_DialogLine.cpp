// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderNode_DialogLine.h"
#include "DialogBuilderNode_PlayerChoice.h"
#include "Kismet2/Kismet2NameValidators.h"
#include "Decorator/OrionDecorator.h"
#include "DialogBuilderGraph.h"
#include "DialogBuilderSetting.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "DialogComponent.h"


#define LOCTEXT_NAMESPACE "DialogSystemGraphNode"


UDialogBuilderNode_DialogLine::UDialogBuilderNode_DialogLine()
{
	bCanSkipDialogLine = true;
	bRotateToListener = false;
	SelectionTimeLimit = ESelectionTimeLimit::E_NoTimeLimit;
	TimeLimit = 5.0f;

	ParticipantInfo.NodeColor = GetDefault<UDialogBuilderSetting>()->DialogLineNodeColor;
}

void UDialogBuilderNode_DialogLine::BeginNode()
{
	Super::BeginNode();
	GetOwningDialogGraph()->BeginDialogLine(this);
	GetDialogComponent()->OnDialogUpdated.Broadcast(this);
}

void UDialogBuilderNode_DialogLine::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	FName ParticipantInfoProperty = FName("ParticipantInfo");

	if (ParticipantInfoProperty == GET_MEMBER_NAME_CHECKED(UDialogBuilderNode_DialogLine, ParticipantInfo))
	{
		PropagateParticipantSettings();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
void UDialogBuilderNode_DialogLine::PostLoad()
{
	Super::PostLoad();

	// make sure we have guid for shared participant 
	if (bSharedParticipant && !SharedParticipantGuid.IsValid())
	{
		FDialogSharedParticipantNodeHelper().MakeSureGuidExists(this);
	}
}
void UDialogBuilderNode_DialogLine::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
}

void UDialogBuilderNode_DialogLine::EvaluateNextNode()
{
	if (GetOwningDialogGraph()->bOptionSelectionActive)
	{
		UE_LOG(LogTemp, Log, TEXT("Player in dialog selection mode"));
		return;
	}

	//Check if there is any player options 
	//If found pany player options, halt evaluation node until option has been selected.
	if (EvaluateAnyPlayerOptions())
		return;


	Super::EvaluateNextNode();
}

bool UDialogBuilderNode_DialogLine::EvaluateAnyPlayerOptions()
{
	//Check for any player options
	TArray<UDialogBuilderNode_PlayerChoice*> PlayerChoices;
	if (ChildrenNodes.IsValidIndex(0) && ChildrenNodes[0].IsA(UDialogBuilderNode_PlayerChoice::StaticClass()))
	{
		for (auto& ChildNode : ChildrenNodes)
		{
			if (UDialogBuilderNode_PlayerChoice* PlayerOption = Cast<UDialogBuilderNode_PlayerChoice>(ChildNode))
			{
				if (PlayerOption->DecoratorConditionMet())
				{
					PlayerChoices.AddUnique(PlayerOption);
				}
			}
		}

		// No valid player choices found, end dialog immediately
		if (PlayerChoices.IsEmpty())
		{
			Deinitialize();
			GetOwningDialogGraph()->EndDialog();
			return true; 
		}

		GetOwningDialogGraph()->LatestRootSelectionNode = this;
		GetOwningDialogGraph()->bOptionSelectionActive = true;
		GetOwningDialogGraph()->BeginChoiceSelection(this);
		GetDialogComponent()->OnEnterChoiceSelection.Broadcast(PlayerChoices);
		return true;
	}
	return false;
}

#if WITH_EDITOR
void UDialogBuilderNode_DialogLine::MakeParticipantShareable(FString ShareName)
{
	if (!DialogGraph)
		return;
	SharedParticipantIdx = INDEX_NONE;
	TArray<int32> Remap;
	for (int32 idx = 0; idx < DialogGraph->AllNodes.Num(); idx++)
	{
		if (UDialogBuilderNode_DialogLine* Node = Cast<UDialogBuilderNode_DialogLine>(DialogGraph->AllNodes[idx]))
		{
			if (Node->SharedParticipantIdx != INDEX_NONE || Node == this)
			{
				Node->SharedParticipantIdx = Remap.AddUnique(Node->SharedParticipantIdx) + 1; // Remaps existing index to lowest index available
			}
		}
	}

	bSharedParticipant = true;
	SharedParticipantName = ShareName;
	SharedParticipantGuid = FGuid::NewGuid();
}

void UDialogBuilderNode_DialogLine::UnshareParticipant()
{
	bSharedParticipant = false;
	SharedParticipantIdx = INDEX_NONE;
	SharedParticipantName.Empty();
	SharedParticipantGuid.Invalidate();

}

void UDialogBuilderNode_DialogLine::UseSharedParticipant(const UDialogBuilderNode_DialogLine* Node)
{
	if (Node == this || Node == nullptr)
	{
		return;
	}

	FScopedTransaction Transaction(LOCTEXT("UseSharedParticipant", "Use Shared Participant"));

	Modify();

	bSharedParticipant = Node->bSharedParticipant;
	SharedParticipantName = Node->SharedParticipantName;
	SharedParticipantGuid = Node->SharedParticipantGuid;
	CopyParticipantSettings(Node);
	OnSharedParticipantChanged.Broadcast();
}

void UDialogBuilderNode_DialogLine::CopyParticipantSettings(const UDialogBuilderNode_DialogLine* SrcNode)
{
	ParticipantInfo = SrcNode->ParticipantInfo;
	SharedParticipantIdx = SrcNode->SharedParticipantIdx;
	SharedParticipantName = SrcNode->SharedParticipantName;
	SharedParticipantGuid = SrcNode->SharedParticipantGuid;
}

void UDialogBuilderNode_DialogLine::PropagateParticipantSettings()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> DialogGraphDataArray;
	AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UDialogBuilderGraph::StaticClass()), DialogGraphDataArray);

	for (const FAssetData& AssetData : DialogGraphDataArray)
	{
		UDialogBuilderGraph* DialogGraphAsset = Cast<UDialogBuilderGraph>(AssetData.GetAsset());
		if (DialogGraphAsset)
		{
			for (int32 idx = 0; idx < DialogGraphAsset->AllNodes.Num(); idx++)
			{
				if (UDialogBuilderNode_DialogLine* Node = Cast<UDialogBuilderNode_DialogLine>(DialogGraphAsset->AllNodes[idx]))
				{
					if (Node->SharedParticipantIdx != INDEX_NONE && Node->SharedParticipantGuid == SharedParticipantGuid)
					{
						Node->Modify();
						Node->CopyParticipantSettings(this);
					}
				}
			}
		}
	}

}

FText UDialogBuilderNode_DialogLine::GetNodeTitle() const
{
    return ParticipantInfo.ParticipantName.IsEmpty() ? FText::FromString("Participant None") : ParticipantInfo.ParticipantName;
}

void UDialogBuilderNode_DialogLine::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}

FText UDialogBuilderNode_DialogLine::GetNodeDescription() const
{
	return DialogLineData.Line;
}
#endif


void IDialogNodeSharedDataHelper::UpdateSharedData(UDialogBuilderNode_DialogLine* Node, TSharedPtr<INameValidatorInterface> NameValidator)
{
	if (!Node || !Node->DialogGraph)
		return;
	// get all other dialog line nodes
	TArray<UDialogBuilderNode_DialogLine*> DialogLineNodes;

	UDialogBuilderGraph* ParentGraph = Node->DialogGraph;
	for (auto& DialogBuilderNode : ParentGraph->AllNodes)
	{
		if (UDialogBuilderNode_DialogLine* DialogLineNode = Cast<UDialogBuilderNode_DialogLine>(DialogBuilderNode))
		{
			DialogLineNodes.AddUnique(DialogLineNode);
		}
	}

	// check if there is other node that can provide us with data
	for (TArray<UDialogBuilderNode_DialogLine*>::TIterator It(DialogLineNodes); It; ++It)
	{
		UDialogBuilderNode_DialogLine* OtherNode = *It;
		if (OtherNode != Node &&
			CheckIfHasDataToShare(OtherNode) &&
			CheckIfNodesShouldShareData(Node, OtherNode))
		{
			// use shared data of that node (to make sure everything is linked up properly)
			ShareData(Node, OtherNode);
			break;
		}
	}

	// check if our shared rule name is original
	if (NameValidator->FindValidString(AccessShareDataName(Node)) != EValidatorResult::Ok)
	{
		// rename all shared rules name in nodes that should share same name
		for (TArray<UDialogBuilderNode_DialogLine*>::TIterator It(DialogLineNodes); It; ++It)
		{
			UDialogBuilderNode_DialogLine* OtherNode = *It;
			if (OtherNode != Node &&
				CheckIfNodesShouldShareData(Node, OtherNode))
			{
				AccessShareDataName(OtherNode) = AccessShareDataName(Node);
			}
		}
	}
}

void IDialogNodeSharedDataHelper::MakeSureGuidExists(UDialogBuilderNode_DialogLine* Node)
{
	if (!Node || !Node->DialogGraph)
		return;

	UDialogBuilderGraph* CurrentGraph = Node->DialogGraph;
	for (int32 idx = 0; idx < CurrentGraph->AllNodes.Num(); idx++)
	{
		if (UDialogBuilderNode_DialogLine* OtherNode = Cast<UDialogBuilderNode_DialogLine>(CurrentGraph->AllNodes[idx]))
		{
			if (OtherNode != Node &&
				CheckIfNodesShouldShareData(Node, OtherNode))
			{
				AccessShareDataName(Node) = AccessShareDataName(OtherNode);
			}
		}
	}

	if (!AccessShareDataGuid(Node).IsValid())
	{
		AccessShareDataGuid(Node) = FGuid::NewGuid();
	}
}

bool FDialogSharedParticipantNodeHelper::CheckIfNodesShouldShareData(const UDialogBuilderNode_DialogLine* NodeA, const UDialogBuilderNode_DialogLine* NodeB)
{
	return NodeA->bSharedParticipant && NodeB->bSharedParticipant && NodeA->SharedParticipantGuid == NodeB->SharedParticipantGuid;
}

bool FDialogSharedParticipantNodeHelper::CheckIfHasDataToShare(const UDialogBuilderNode_DialogLine* Node)
{
	return Node->SharedParticipantIdx != INDEX_NONE;
}

void FDialogSharedParticipantNodeHelper::ShareData(UDialogBuilderNode_DialogLine* NodeWhoWantsToShare, const UDialogBuilderNode_DialogLine* ShareFrom)
{
	NodeWhoWantsToShare->UseSharedParticipant(ShareFrom);
}

FString& FDialogSharedParticipantNodeHelper::AccessShareDataName(UDialogBuilderNode_DialogLine* Node)
{
	return Node->SharedParticipantName;
}

FGuid& FDialogSharedParticipantNodeHelper::AccessShareDataGuid(UDialogBuilderNode_DialogLine* Node)
{
	return Node->SharedParticipantGuid;
}


#undef LOCTEXT_NAMESPACE

