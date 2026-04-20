// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "DialogBuilderNode_DialogSequence.h"
#include "DialogBuilderNode_PlayerChoice.h"
#include "Decorator/OrionDecorator.h"
#include "DialogBuilderGraph.h"
#include "DialogSequence.h"
#include "DialogBuilderSetting.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Animation/AnimMontage.h"
#include "Sound/SoundBase.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "DialogComponent.h"
#include "DialogStage.h"

#define LOCTEXT_NAMESPACE "DialogNode_Sequence"



UDialogBuilderNode_DialogSequence::UDialogBuilderNode_DialogSequence()
{
}

void UDialogBuilderNode_DialogSequence::BeginNode()
{
	Super::BeginNode();

	if (!ShouldPlaySequence())
	{
		EvaluateNextNode();
		return;
	}

	GetOwningDialogGraph()->BeginDialogSequence(this);
}

void UDialogBuilderNode_DialogSequence::EvaluateNextNode()
{
	Super::EvaluateNextNode();
}

bool UDialogBuilderNode_DialogSequence::ShouldPlaySequence()
{
	return DialogSequence && DialogStage && GetSequenceDuration() > KINDA_SMALL_NUMBER;
}

void UDialogBuilderNode_DialogSequence::EnsureSequenceCreated()
{
	if (!DialogSequence)
	{
		DialogSequence = NewObject<UDialogSequence>(this, NAME_None, RF_Transactional);
		DialogSequence->OwningDialogGraph = GetOwningDialogGraph();	
		DialogSequence->MovieScene = NewObject<UMovieScene>(DialogSequence, FName("DialogSequence"), RF_Transactional);
		DialogSequence->MovieScene->SetDisplayRate(FFrameRate(30, 1));
		DialogSequence->RefreshSequence();
	}

	
}

float UDialogBuilderNode_DialogSequence::GetSequenceDuration()
{
	float Duration = 0.f;
	if (DialogSequence)
	{
		FFrameRate TickResolution = DialogSequence->MovieScene->GetTickResolution();
		float SequenceDuration = DialogSequence->MovieScene->GetPlaybackRange().GetUpperBoundValue() / TickResolution;
		Duration = FMath::Max(Duration, SequenceDuration);
	}
	return Duration;
}

void UDialogBuilderNode_DialogSequence::UseDialogStage(UDialogStage* InDialogStage)
{

	DialogStageToUse = InDialogStage;

	UpdateDialogStageData();
	Modify();
}

void UDialogBuilderNode_DialogSequence::UpdateDialogStageData()
{
	if (!DialogStageToUse)
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogStageToUse is null, cannot update DialogStage data"));
		DialogStage = nullptr;
		return;
	}

	if (!DialogStage)
	{
		DialogStage = NewObject<UDialogStage>(this, NAME_None, RF_Transactional);
		DialogStage->OwningDialogGraph = GetOwningDialogGraph();
	}

	DialogStage->Name = DialogStageToUse->Name;
	DialogStage->Location = DialogStageToUse->Location;
	DialogStage->Rotation = DialogStageToUse->Rotation;

	//Populate Dialog Definition Slots
	{
		const int32 SourceCount = DialogStageToUse->Slots.Num();
		const int32 SlotCount = DialogStage->Slots.Num();

		//remove possessable if there is excess slots
		if (SlotCount > SourceCount)
		{
			for (int32 Index = SlotCount - 1; Index >= SourceCount; --Index)
			{
				if (UDialogSequenceSlot* Slot = DialogStage->Slots[Index])
				{
					Slot->DialogDefinition = nullptr;
					//TODO: for now, dont remove possessable, as changing empty dialog set will delete all your progress
					//DialogSequence->MovieScene->RemovePossessable(Slot->ID);
				}

			}
		}
		
		if (SlotCount < SourceCount)
		{
			DialogStage->Slots.SetNum(SourceCount);
		}
		for (int32 idx = 0; idx < SourceCount; ++idx)
		{
			UDialogSequenceSlot* Source = DialogStageToUse->Slots[idx];
			if (!Source)
			{
				DialogStage->Slots[idx] = nullptr;
				continue;
			}

			UDialogSequenceSlot* Target = DialogStage->Slots[idx];
			if (!Target)
			{
				Target = NewObject<UDialogSequenceSlot>(DialogStage, NAME_None, RF_Transactional);
			}

			Target->OwningDialogGraph = GetOwningDialogGraph();
			Target->DialogDefinition = Source->DialogDefinition;
			Target->SlotLocation = Source->SlotLocation;
			Target->SlotRotation = Source->SlotRotation	;

			DialogStage->Slots[idx] = Target;
		}
	}

	//Populate Camera Slots
	/*{
		const int32 SourceCount = DialogStageToUse->CameraSlots.Num();
		DialogStage->CameraSlots.SetNum(SourceCount);
		for (int32 idx = 0; idx < SourceCount; ++idx)
		{
			UDialogSequenceSlot* Source = DialogStageToUse->CameraSlots[idx];
			if (!Source)
			{
				DialogStage->CameraSlots[idx] = nullptr;
				continue;
			}

			UDialogSequenceSlot* Target = DialogStage->CameraSlots[idx];
			if (!Target)
			{
				Target = NewObject<UDialogSequenceSlot>(DialogStage, NAME_None, RF_Transactional);
			}

			Target->OwningDialogGraph = GetOwningDialogGraph();
			Target->ActorTag = Source->ActorTag;
			Target->DialogDefinition = Source->DialogDefinition;
			Target->SlotPlacement = Source->SlotPlacement;

			DialogStage->CameraSlots[idx] = Target;
		}
	}*/
}




#if WITH_EDITOR

FText UDialogBuilderNode_DialogSequence::GetNodeTitle() const
{
    return NodeDisplayName.IsEmpty() ? FText::FromString("Dialog Sequence") : FText::FromString(NodeDisplayName);
}

void UDialogBuilderNode_DialogSequence::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}

FText UDialogBuilderNode_DialogSequence::GetNodeDescription() const
{
    return FText::FromString("");
}


void UDialogBuilderNode_DialogSequence::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UDialogBuilderNode_DialogSequence::PostLoad()
{
	Super::PostLoad();
	EnsureSequenceCreated();
}

#endif

void UDialogBuilderNode_DialogSequence::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
}
#undef LOCTEXT_NAMESPACE

