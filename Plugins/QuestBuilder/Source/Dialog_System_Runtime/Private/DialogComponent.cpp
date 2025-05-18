// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogComponent.h"
#include "Dialog.h"
#include "Decorator/OrionDecorator.h"
#include "Event/OrionEvent.h"

#include "UObject/UObjectIterator.h"
#include "DialogSaveObject.h"
#include "DialogBuilderGraph.h"
#include "DialogBuilderNode_State.h"
#include "DialogBuilderNode_Root.h"
#include "DialogBuilderNode_Objective.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Controller.h"
#include "DialogData.h"




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
	NodeMap.Empty();
	bDialogLoaded = false;
	//Set Dependency
	OwningController = GetOwningController();

	OnDialogObjectiveUpdated.AddDynamic(this, &UDialogComponent::DialogObjectiveUpdated);
	OnStartNavigateDialog.AddDynamic(this, &UDialogComponent::StartNavigatingDialog);
	OnDialogFailed.AddDynamic(this, &UDialogComponent::DialogFailed);
	OnDialogCompleted.AddDynamic(this, &UDialogComponent::DialogCompleted);
	OnDialogAdded.AddDynamic(this, &UDialogComponent::DialogAdded);
	OnDialogUnlocked.AddDynamic(this, &UDialogComponent::DialogUnlocked);

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
		EvaluateDialogSaveLoadState(NewDialogGraph);

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
			for (auto& Node : Dialog->AllNodes)
			{
				NodeMap.Emplace(Node->ID, Node);
			}
		}
	}
}

UDialogBuilderGraph* UDialogComponent::GetDialogGraphInstance(UDialogBuilderGraph* DialogGraph) const
{
	if (DialogGraph == nullptr)
		return nullptr;
	
	for (auto& Inst : DialogGraphs)
	{
		if (Inst && Inst->GetClass()->IsChildOf(DialogGraph->GetClass()))
		{
			return Inst;
		}
	}
	return nullptr;
}

class UDialogBuilderGraph* UDialogComponent::MakeDialogGraphInstance(UDialogBuilderGraph* DialogGraphTemplate)
{
	if (IsValid(DialogGraphTemplate))
	{
		//Duplicate the dialog graph
		UDialogBuilderGraph* NewDialogGraph = Cast<UDialogBuilderGraph>(StaticDuplicateObject(DialogGraphTemplate, this, NAME_None, RF_Transactional));
		NewDialogGraph->SetFlags(RF_Transient | RF_DuplicateTransient);
		return NewDialogGraph;
		

	}
	return nullptr;
}

void UDialogComponent::EvaluateDialogSaveLoadState(UDialogBuilderGraph* DialogGraph)
{
	for (auto& Dialog : DialogGraph->DialogList)
	{
		//check if dialog has been loaded
		if (!bDialogLoaded)
		{
			LockDialog(Dialog);
		}
	}
}

void UDialogComponent::ActivateQuest(FName ID)
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
			RestartQuest(Dialog->ID);
		}
	}
}

void UDialogComponent::UnlockQuest(FName ID)
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

void UDialogComponent::RestartQuest(FName ID, FName NodeID, bool bStartFromNodeID)
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

				//reset task data from the founded node
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
			//reset all task data
			for (auto& Node : Dialog->AllNodes)
			{
				if (UDialogBuilderNode_Objective* ObjectiveNode = Cast<UDialogBuilderNode_Objective>(Node))
				{
				}
			}
			Dialog->VisitedNodeIDs.Empty();
			Dialog->CurrentNode = Dialog->RootNodes[0];
		}
		

		BeginNode(Dialog);
	}
}

void UDialogComponent::FailQuest(FName ID)
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

bool UDialogComponent::IsQuestActiveFromNodeID(FName NodeID) const
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

bool UDialogComponent::IsQuestActiveFromDialogID(FName ID) const
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
	for (const auto& It : DialogMap)
	{
		FName Key = It.Key;
		UDialog* Dialog = It.Value;

		EvaluateDialogCondition(Dialog);

		if(Dialog->DialogState == EDialogState::E_Active)
		{
			if (Dialog->CurrentNode != nullptr)
			{
				if (UDialogBuilderNode_Objective* ObjectiveNode = Cast<UDialogBuilderNode_Objective>(Dialog->CurrentNode))
				{
				}
			}
			else if (Dialog->CurrentNode == nullptr)
			{
				RestartQuest(Dialog->ID);
			}
		}
		
		
	}
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
		DialogParam->CurrentNode->BeginEvents(EEventRunType::E_End);
		NextNode->BeginEvents(EEventRunType::E_Start);

		//Go to next node if exist or condition met
		GoToNextNode(DialogParam, NextNode);
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
				ActivateQuest(DialogParam->ID);
				return;
			case EDialogState::E_Fail:
				FailQuest(DialogParam->ID);
				return;
			case EDialogState::E_Complete:
				CompleteDialog(DialogParam->ID);
				return;
			case EDialogState::E_Unlocked:
				UnlockQuest(DialogParam->ID);
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
