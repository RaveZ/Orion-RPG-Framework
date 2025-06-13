// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderNode_DialogLine.h"
#include "Kismet2/Kismet2NameValidators.h"
#include "DialogBuilderGraph.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "DialogComponent.h"


#define LOCTEXT_NAMESPACE "DialogSystemGraphNode"

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
}

void UDialogBuilderNode_DialogLine::CopyParticipantSettings(const UDialogBuilderNode_DialogLine* SrcNode)
{
	ParticipantTag = SrcNode->ParticipantTag;
	ParticipantName = SrcNode->ParticipantName;
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

void UDialogBuilderNode_DialogLine::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UDialogBuilderNode_DialogLine, ParticipantTag) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UDialogBuilderNode_DialogLine, ParticipantName))
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

#if WITH_EDITOR

FText UDialogBuilderNode_DialogLine::GetNodeTitle() const
{
    return FText::FromName(ID).IsEmpty() ? FText::FromString(UDialogBuilderNode::GetShortTypeName(this)) : FText::FromName(ID);
}

void UDialogBuilderNode_DialogLine::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}

FText UDialogBuilderNode_DialogLine::GetNodeDescription() const
{
    return Description;
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

