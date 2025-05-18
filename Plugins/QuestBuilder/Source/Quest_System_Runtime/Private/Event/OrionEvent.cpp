// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Event/OrionEvent.h"
#include "UObject/UObjectIterator.h"
#include "Quest_System_Runtime.h"
#include "QuestComponent.h"
#include "Quest.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderNode.h"
#include "QuestBuilderNode_Objective.h"
#include "QuestBuilderNode_Root.h"
#include "QuestBuilderFunctionLibrary.h"	
#include "AssetRegistry/AssetRegistryModule.h"


UOrionEvent::UOrionEvent()
{
}

void UOrionEvent::BeginEvent_Implementation(AController* OwnerController, APawn* ControlledPawn)
{
}

FString UOrionEvent::GetNodeDisplayText_Implementation() const
{
	return "Node Description";
}

UQuestComponent* UOrionEvent::GetQuestComponent() const
{
	if (GetWorld())
	{
		return UQuestBuilderFunctionLibrary::GetQuestComponent(GetWorld());
	}
	return nullptr;
}


TArray<FName> UOrionEvent::GetAllQuestID()
{
	TArray<FName> QuestIDs;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> QuestGraphDataArray;
	AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UQuestBuilderGraph::StaticClass()), QuestGraphDataArray);

	for (const FAssetData& AssetData : QuestGraphDataArray)
	{
		UQuestBuilderGraph* QuestGraphAsset = Cast<UQuestBuilderGraph>(AssetData.GetAsset());
		if (QuestGraphAsset)
		{
			for (UQuest* QuestObj : QuestGraphAsset->QuestList)
			{
				QuestIDs.Add(QuestObj->ID);
			}
		}
	}

	return QuestIDs;
}

TArray<FName> UOrionEvent::GetAllNodeID()
{
	TArray<FName> NodeIDs;
	if (bUseQuestID)
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

		TArray<FAssetData> QuestGraphDataArray;
		AssetRegistryModule.Get().GetAssetsByClass(FTopLevelAssetPath(UQuestBuilderGraph::StaticClass()), QuestGraphDataArray);

		for (const FAssetData& AssetData : QuestGraphDataArray)
		{
			UQuestBuilderGraph* QuestGraphAsset = Cast<UQuestBuilderGraph>(AssetData.GetAsset());
			if (QuestGraphAsset)
			{
				for (UQuest* QuestObj : QuestGraphAsset->QuestList)
				{
					if (QuestObj->ID != QuestID) continue;
					for (UQuestBuilderNode* Node : QuestObj->AllNodes)
					{
						if (Node)
						{
							if (UQuestBuilderNode_Root* RootNode = Cast<UQuestBuilderNode_Root>(Node)) continue;
							NodeIDs.Emplace(Node->ID);
						}
					}
				}
			}
		}
	}
	else
	{
	}
	return NodeIDs;
	
}

FString UOrionEvent::GetNodeName()
{
	return NodeName.Len() ? NodeName : GetShortTypeName(this);
}

FString UOrionEvent::GetShortTypeName(UObject* Ob)
{
	if ((Ob == nullptr) || (Ob->GetClass() == nullptr))
	{
		return TEXT("None");
	}

	FString TypeDesc = Ob->GetClass()->GetName();

	if (Ob->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{

		TypeDesc = Ob->GetClass()->GetName().LeftChop(2);
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

UWorld* UOrionEvent::GetWorld() const
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

#if WITH_EDITOR
void UOrionEvent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property)
	{
		FName PropertyName = PropertyChangedEvent.Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UOrionEvent, QuestID))
		{
			NodeID = NAME_None;
		}
	}
	
}
#endif
