// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogComponent.h"
#include "Dialog.h"
#include "Decorator/OrionDecorator.h"
#include "Event/OrionEvent.h"
#include "UObject/UObjectIterator.h"
#include "DialogSaveObject.h"
#include "DialogBuilderGraph.h"
#include "DialogBuilderNode_State.h"
#include "DialogBuilderNode_Checkpoint.h"
#include "DialogBuilderNode_Root.h"
#include "DialogBuilderNode_Speaker.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Controller.h"
#include "DialogData.h"
#include "AssetRegistry/AssetRegistryModule.h"




#define LOCTEXT_NAMESPACE "DialogComponent"
// Sets default values for this component's properties
UDialogComponent::UDialogComponent()
{
	SetIsReplicatedByDefault(true);
	SetComponentTickEnabled(true);
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UDialogComponent::BeginPlay()
{
	Super::BeginPlay();
	DialogMap.Empty();
	//Set Dependency
	OwningController = GetOwningController();

	OnDialogObjectiveUpdated.AddDynamic(this, &UDialogComponent::DialogObjectiveUpdated);
	OnStartNavigateDialog.AddDynamic(this, &UDialogComponent::StartNavigatingDialog);
	OnDialogFailed.AddDynamic(this, &UDialogComponent::DialogFailed);
	OnDialogCompleted.AddDynamic(this, &UDialogComponent::DialogCompleted);
	OnDialogAdded.AddDynamic(this, &UDialogComponent::DialogAdded);
	OnDialogUnlocked.AddDynamic(this, &UDialogComponent::DialogUnlocked);

#if WITH_ORION_COMBAT_FRAMEWORK

#else

#endif

}


// Called every frame
void UDialogComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UDialogComponent::BeginDialog(UDialogBuilderGraph* DialogAsset)
{
	if (DialogAsset == nullptr)
	{
		return false;
	}

	bool bSuccess = false;

	UDialogBuilderGraph* NewDialogGraph = MakeDialogGraphInstance(DialogAsset);
	if (NewDialogGraph)
	{
		CacheDialogGraphDependencies(NewDialogGraph);

		for (UDialog* Dialog : NewDialogGraph->DialogList)
		{
			EvaluateDialogCondition(Dialog);
		}
		bSuccess = true;
	}
	
	return bSuccess;
}

bool UDialogComponent::BeginMultipleDialog(TArray<UDialogBuilderGraph*> DialogAssets)
{
	if (DialogAssets.IsEmpty())
	{
		return false;
	}


	for (UDialogBuilderGraph* DialogAsset : DialogAssets)
	{
		BeginDialog(DialogAsset);
	}
	bool bSuccess = true;
	return bSuccess;
}

void UDialogComponent::CacheDialogGraphDependencies(UDialogBuilderGraph* DialogGraph)
{
	if (DialogGraph)
	{
		DialogGraph->OwningController = OwningController;
		DialogGraph->DialogComponent = this;
		DialogGraph->OwningPawn = GetOwningPawn();
		

		//Add DialogGraph to array
		DialogGraphs.Add(DialogGraph);

		for (auto& Dialog : DialogGraph->DialogList)
		{
			DialogMap.Emplace(Dialog->ID, Dialog);
		}
	}
}

UDialogBuilderGraph* UDialogComponent::GetDialogGraphInstance(UDialogBuilderGraph* DialogGraph) const
{
	if (DialogGraph == nullptr)
		return nullptr;
	
	for (auto& Inst : DialogGraphs)
	{
		if (Inst && Inst->ID == DialogGraph->ID)
		{
			return Inst;
		}
	}
	return nullptr;
}

class UDialogBuilderGraph* UDialogComponent::MakeDialogGraphInstance(UDialogBuilderGraph* DialogGraphTemplate)
{
	if (IsValid(GetDialogGraphInstance(DialogGraphTemplate)))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s Already Registered. Use RestartDialog() instead. "), *DialogGraphTemplate->CategoryDisplayName);
		return nullptr;
	}

	if (IsValid(DialogGraphTemplate))
	{
		//Duplicate the dialog graph
		UDialogBuilderGraph* NewDialogGraph = Cast<UDialogBuilderGraph>(StaticDuplicateObject(DialogGraphTemplate, this, NAME_None, RF_Transactional));
		return NewDialogGraph;
	}

	return nullptr;
}

void UDialogComponent::ActivateDialog(FName ID)
{
	if (UDialog* Dialog = DialogMap.FindRef(ID))
	{
		Dialog->DialogState = EDialogState::E_Active;
		if (Dialog->CurrentNode != nullptr)
		{
			BeginNode(Dialog);
		}
		else if (Dialog->CurrentNode == nullptr)
		{
			OnDialogAdded.Broadcast(Dialog);
			RestartDialog(Dialog->ID);
		}
	}
}

void UDialogComponent::UnlockDialog(FName ID)
{
	if (UDialog* Dialog = DialogMap.FindRef(ID))
	{
		Dialog->DialogState = EDialogState::E_Unlocked;
		OnDialogUnlocked.Broadcast(Dialog);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogComponent : Dialog %s not found or null"), *ID.ToString());
	}
}

void UDialogComponent::LockDialog(UDialog* InDialog, FName ID)
{
	UDialog* Dialog = nullptr;
	if (InDialog)
		Dialog = InDialog;
	else
		Dialog = DialogMap.FindRef(ID);

	if (Dialog)
	{
		Dialog->DialogState = EDialogState::E_Locked;
		Dialog->CurrentNode = nullptr;
		Dialog->VisitedNodeIDs.Empty();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogComponent : Dialog %s not found or null"), *ID.ToString());
	}
}

void UDialogComponent::RestartDialog(FName ID, FName NodeID, bool bStartFromNodeID)
{
	if (UDialog* Dialog = DialogMap.FindRef(ID))
	{
		Dialog->DialogState = EDialogState::E_Active;


		bool bNodeFound = false;
		if (NodeID.IsNone())
			bStartFromNodeID = false;

		if (bStartFromNodeID)
		{	
			if (UDialogBuilderNode* FoundedNode = Dialog->NodeMap.FindRef(NodeID))
			{
				bNodeFound = true;
				//reset visited node children where the node started from
				bool bCanDelete = false;
				for (int i = 0; i < Dialog->VisitedNodeIDs.Num(); i++)
				{
					if (Dialog->VisitedNodeIDs[i] == NodeID)
					{
						bCanDelete = true;
					}
					if (bCanDelete)
					{
						Dialog->VisitedNodeIDs.RemoveAt(i);
						i--;//ensure the next element not skipped
					}
				}

				TArray<UDialogBuilderNode*> NodeStack;
				NodeStack.Add(FoundedNode);
				while (NodeStack.Num() > 0)
				{
					UDialogBuilderNode* CurrentNodeStack = NodeStack.Pop();
					// Traverse child node
					for (int i = 0; i < CurrentNodeStack->ChildrenNodes.Num(); i++)
					{
						NodeStack.Add(CurrentNodeStack->ChildrenNodes[i]);
					}
				}

				Dialog->CurrentNode = FoundedNode;
			}
		}
		

		if(!bNodeFound)
		{
			
			Dialog->VisitedNodeIDs.Empty();
			if (Dialog->RootNodes.IsValidIndex(0))
			{
				Dialog->CurrentNode = Dialog->RootNodes[0];
			}
		}
		

		BeginNode(Dialog);
	}
}

void UDialogComponent::FailDialog(FName ID)
{
	if (UDialog* Dialog = DialogMap.FindRef(ID))
	{
		if (Dialog->DialogState != EDialogState::E_Fail)
		{
			Dialog->DialogState = EDialogState::E_Fail;
			OnDialogFailed.Broadcast(Dialog);
		}
		Dialog->CurrentNode = nullptr;

	}
}

void UDialogComponent::CompleteDialog(FName ID)
{
	if (UDialog* Dialog = DialogMap.FindRef(ID))
	{
		if (Dialog->DialogState != EDialogState::E_Complete)
		{
			Dialog->DialogState = EDialogState::E_Complete;
			OnDialogCompleted.Broadcast(Dialog);
		}
		Dialog->CurrentNode = nullptr;

	}
}


void UDialogComponent::StartNavigateDialog(UDialog* InDialog)
{
	if (InDialog)
	{
		CurrentNavigatedDialog = InDialog;
		OnStartNavigateDialog.Broadcast(InDialog);
	}
}



APawn* UDialogComponent::GetOwningPawn() const
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

APlayerController* UDialogComponent::GetOwningController() const
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

UDialog* UDialogComponent::FindDialogFromNodeID(FName NodeIDParam) const
{
	UDialog* FoundedDialog = nullptr;
	for (const auto& It : DialogMap)
	{
		FName Key = It.Key;
		UDialog* Dialog = It.Value;

		if (UDialogBuilderNode* DialogNode = Dialog->NodeMap.FindRef(NodeIDParam))
		{
			FoundedDialog = Dialog;
		}
	}

	return FoundedDialog;

}

UDialog* UDialogComponent::FindDialog(FName DialogIDParam) const
{
	if (UDialog* FoundedDialog = DialogMap.FindRef(DialogIDParam))
	{
		return FoundedDialog;
	}

	UE_LOG(LogTemp, Warning, TEXT("Dialog Not Found"));
	return nullptr;
}

bool UDialogComponent::IsDialogActiveFromNodeID(FName NodeID) const
{
	if (UDialog* FoundedDialog = FindDialogFromNodeID(NodeID))
	{
		if (FoundedDialog->DialogState == EDialogState::E_Active)
		{
			return true;
		}
	}

	return false;
}

bool UDialogComponent::IsDialogActiveFromDialogID(FName ID) const
{
	if (UDialog* FoundedDialog = DialogMap.FindRef(ID))
	{
		if (FoundedDialog->DialogState == EDialogState::E_Active)
		{
			return true;
		}
	}

	return false;
}

void UDialogComponent::UpdateDialog()
{
    TArray<FName> DialogKeys;
    DialogMap.GetKeys(DialogKeys);

    for (int32 i = 0; i < DialogKeys.Num(); i++)
    {
		FName Key = DialogKeys[i];
		UDialog* Dialog = DialogMap.FindRef(Key);

		EvaluateDialogCondition(Dialog);

		if (Dialog->DialogState == EDialogState::E_Active)
		{
			if (Dialog->CurrentNode != nullptr)
			{
				
				if (UDialogBuilderNode_Checkpoint* CheckpointNode = Cast<UDialogBuilderNode_Checkpoint>(Dialog->CurrentNode))
				{
					//evaluate next node condition
					EvaluateNextNodeCondition(Dialog);

				}
			}
			else if (Dialog->CurrentNode == nullptr)
			{
				RestartDialog(Dialog->ID);
			}
		}
    }
}

void UDialogComponent::ResetAllDialog()
{
	for (int32 j = DialogGraphs.Num() - 1; j >= 0; --j)
	{
		if (DialogGraphs.IsValidIndex(j))
		{
			DialogGraphs[j]->Deinitialize();
			for (int32 i = DialogGraphs[j]->DialogList.Num() - 1; i >= 0; --i)
			{
				if (DialogGraphs[j]->DialogList.IsValidIndex(i))
				{
					DialogGraphs[j]->DialogList[i]->Deinitialize();
					DialogGraphs[j]->DialogList.RemoveAt(i);
				}
			}
			DialogGraphs.RemoveAt(j);
		}
		
	}
	DialogMap.Empty();
	CurrentNavigatedDialog = nullptr;
}

void UDialogComponent::EvaluateNextNodeCondition(UDialog* DialogParam)
{
	UDialogBuilderNode* NextNode = nullptr;
	for (auto& Node : DialogParam->CurrentNode->ChildrenNodes)
	{
		if (NextNode == nullptr && Node->Decorators.IsEmpty())
		{
			NextNode = Node;
		}
		else if (!Node->Decorators.IsEmpty())
		{
			int SumConditionMet = 0;
			for (auto& Condition : Node->Decorators)
			{
				bool bConditionMet = Condition && Condition->InvertCondition ? !Condition->PerformConditionCheck(OwningController, GetOwningPawn()) : Condition->PerformConditionCheck(OwningController, GetOwningPawn());
				if (bConditionMet)
				{
					SumConditionMet++;
				}
			}
			if (SumConditionMet >= Node->Decorators.Num())
			{
				NextNode = Node;
				//break loop to ensure the met condition doesn't get overrided
				break;
			}
		}
	}

	if (NextNode)
	{
		UDialogBuilderNode* CachedCurrentNode = DialogParam->CurrentNode;

		CachedCurrentNode->BeginEvents(EEventRunType::E_End);

		//Check if the current node has changed after the events. it means we dont need to go to the next node anymore
		if (CachedCurrentNode != DialogParam->CurrentNode)
		{
			//if the current node is not the same as the cached current node, it means we are in a state node
			//and we need to start the events again
			if (UDialogBuilderNode_State* StateNode = Cast<UDialogBuilderNode_State>(DialogParam->CurrentNode))
			{
				StateNode->BeginEvents(EEventRunType::E_Start);
			}
		}
		else if (DialogParam->CurrentNode == CachedCurrentNode)	
		{
			DialogParam->CurrentNode->BeginEvents(EEventRunType::E_Start);
		}
		else
		{
			GoToNextNode(DialogParam, NextNode);
			NextNode->BeginEvents(EEventRunType::E_Start);
		}
	}
	
}

void UDialogComponent::EvaluateDialogCondition(UDialog* DialogParam)
{
	if (DialogParam->DialogState != EDialogState::E_Locked)
		return;

	int SumConditionMet = 0;
	for (auto& Condition : DialogParam->Decorators)
	{
		bool bConditionMet = Condition && Condition->InvertCondition ? !Condition->PerformConditionCheck(OwningController, GetOwningPawn()) : Condition->PerformConditionCheck(OwningController, GetOwningPawn());
		if (bConditionMet)
		{
			SumConditionMet++;
		}
	}
	EvaluateDialogState(DialogParam, SumConditionMet);
	
}

void UDialogComponent::EvaluateDialogState(UDialog* DialogParam, int SumConditionMet)
{
	if (DialogParam->DialogState == EDialogState::E_Complete || DialogParam->DialogState == EDialogState::E_Fail)
		return;

	if (SumConditionMet >= DialogParam->Decorators.Num())
	{
		switch (DialogParam->DesiredDialogState)
		{
			case EDialogState::E_Active:
				ActivateDialog(DialogParam->ID);
				return;
			case EDialogState::E_Fail:
				FailDialog(DialogParam->ID);
				return;
			case EDialogState::E_Complete:
				CompleteDialog(DialogParam->ID);
				return;
			case EDialogState::E_Unlocked:
				UnlockDialog(DialogParam->ID);
				return;
		}
		
	}
}

void UDialogComponent::GoToNextNode(UDialog* DialogParam, UDialogBuilderNode* NodeParam)
{
	DialogParam->CurrentNode = NodeParam;
	BeginNode(DialogParam);
}

void UDialogComponent::BeginNode(UDialog* DialogParam)  
{  
   if (DialogParam->CurrentNode == nullptr)  
   {  
       UE_LOG(LogTemp, Warning, TEXT("BeginNode: CurrentNode is null for Dialog ID: %s"), *DialogParam->ID.ToString());  
       return;  
   }  

   //add visited node
   DialogParam->VisitedNodeIDs.AddUnique(DialogParam->CurrentNode->ID);

   if (UDialogBuilderNode_Root* RootNode = Cast<UDialogBuilderNode_Root>(DialogParam->CurrentNode))
   {
	   EvaluateNextNodeCondition(DialogParam);
	   return;
   }

   //InitializeNode
   DialogParam->CurrentNode->Initialize();
}




void UDialogComponent::Save_Implementation(const FString& SaveFileName, int SlotIndex)
{
}

void UDialogComponent::Load_Implementation(const FString& SaveFileName, int SlotIndex) 
{
	
}

bool UDialogComponent::DeleteSave(const FString& SaveName, const int32 Slot)
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveName, 0))
	{
		return false;
	}

	return UGameplayStatics::DeleteGameInSlot(SaveName, Slot);
}




#undef LOCTEXT_NAMESPACE

void UDialogComponent::DialogObjectiveUpdated()
{
	UpdateDialog();
}

void UDialogComponent::StartNavigatingDialog(UDialog* Dialog)
{
	if (Dialog)
	{
        UE_LOG(LogTemp, Log, TEXT("Start Navigating Dialog: %s"), *Dialog->ID.ToString());  
    }  
}

void UDialogComponent::DialogFailed(UDialog* Dialog)
{
	if (Dialog)
	{
		UE_LOG(LogTemp, Log, TEXT("Dialog failed: %s"), *Dialog->ID.ToString());
	}
}

void UDialogComponent::DialogCompleted(UDialog* Dialog)
{
	if (Dialog)
	{
		UE_LOG(LogTemp, Log, TEXT("Dialog Completed: %s"), *Dialog->ID.ToString());
	}
}

void UDialogComponent::DialogAdded(UDialog* Dialog)
{
	if (Dialog)
	{
		UE_LOG(LogTemp, Log, TEXT("Dialog Added: %s"), *Dialog->ID.ToString());
	}
}

void UDialogComponent::DialogUnlocked(UDialog* Dialog)
{
	if (Dialog)
	{
		UE_LOG(LogTemp, Log, TEXT("Dialog Unlocked: %s"), *Dialog->ID.ToString());
	}
}

void UDialogComponent::BeginSave(FString SaveName)
{
	UE_LOG(LogTemp, Verbose, TEXT("Begun saving using save name: %s"), *SaveName);
}

void UDialogComponent::BeginLoad(FString SaveName)
{
	UE_LOG(LogTemp, Verbose, TEXT("Begun loading using save name: %s"), *SaveName);
}

void UDialogComponent::SaveComplete(FString SaveName)
{
	UE_LOG(LogTemp, Verbose, TEXT("Save complete for save name: %s"), *SaveName);
}

void UDialogComponent::LoadComplete(FString SaveName)
{
	UE_LOG(LogTemp, Verbose, TEXT("Load complete for save name: %s"), *SaveName);
}
