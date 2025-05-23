// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "DialogData.h"
#include "OrionRPG.h"
#include "Dialog.h"
#include "DialogBuilderGraph.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderNode_Root.h"
#include "AssetRegistry/AssetRegistryModule.h"

TArray<FName> UDialogHandle::GetAllDialogID()
{
	TArray<FName> DialogIDs;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> DialogGraphDataArray;
	AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UDialogBuilderGraph::StaticClass()), DialogGraphDataArray);

	for (const FAssetData& AssetData : DialogGraphDataArray)
	{
		UDialogBuilderGraph* DialogGraphAsset = Cast<UDialogBuilderGraph>(AssetData.GetAsset());
		if (DialogGraphAsset)
		{
			for (UDialog* DialogObj : DialogGraphAsset->DialogList)
			{
				DialogIDs.Add(DialogObj->ID);
			}
		}
	}

	return DialogIDs;
}

TArray<FName> UDialogHandle::GetAllNodeID()
{
	TArray<FName> NodeIDs;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> DialogGraphDataArray;
	AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UDialogBuilderGraph::StaticClass()), DialogGraphDataArray);

	for (const FAssetData& AssetData : DialogGraphDataArray)
	{
		UDialogBuilderGraph* DialogGraphAsset = Cast<UDialogBuilderGraph>(AssetData.GetAsset());
		if (DialogGraphAsset)
		{
			for (UDialog* DialogObj : DialogGraphAsset->DialogList)
			{
				if (DialogObj->ID != DialogID) continue;
				for (UDialogBuilderNode* Node : DialogObj->AllNodes)
				{
					if (Node)
					{
						if (UDialogBuilderNode_Root* RootNode = Cast<UDialogBuilderNode_Root>(Node)) continue;
						NodeIDs.Emplace(Node->ID);
					}
				}
			}
		}
	}
	
	return NodeIDs;
}

FDialogDataHandle::FDialogDataHandle()
{
}

void FDialogDataHandle::PostSerialize(const FArchive& Ar)
{
	if (DialogDataHandle)
	{
		DialogID = DialogDataHandle->DialogID;
		if (DialogID != NAME_None)
		{
			NodeID = DialogDataHandle->NodeID;
		}
		else
		{
			NodeID = NAME_None;
		}
	}
	else
	{
	}
	
}