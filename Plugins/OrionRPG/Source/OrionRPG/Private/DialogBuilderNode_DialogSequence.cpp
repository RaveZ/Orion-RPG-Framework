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
#include "DialogSet.h"

#define LOCTEXT_NAMESPACE "DialogNode_Sequence"



UDialogBuilderNode_DialogSequence::UDialogBuilderNode_DialogSequence()
{
	
}

void UDialogBuilderNode_DialogSequence::BeginNode()
{
	Super::BeginNode();

	GetOwningDialogGraph()->BeginDialogSequence(this);
}

void UDialogBuilderNode_DialogSequence::EvaluateNextNode()
{
	Super::EvaluateNextNode();
}

void UDialogBuilderNode_DialogSequence::EnsureSequenceCreated()
{
	if (!DialogSequence)
	{
		Modify();
		DialogSequence = NewObject<UDialogSequence>(this, NAME_None, RF_Transactional);
		DialogSequence->OwningDialogGraph = GetOwningDialogGraph();	
		DialogSequence->MovieScene = NewObject<UMovieScene>(DialogSequence, FName("DialogSequence"), RF_Transactional);
		DialogSequence->MovieScene->SetDisplayRate(FFrameRate(30, 1));
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

void UDialogBuilderNode_DialogSequence::UseDialogSet(UDialogSet* InDialogSet)
{

	DialogSetToUse = InDialogSet;

	UpdateDialogSetData();
}

void UDialogBuilderNode_DialogSequence::UpdateDialogSetData()
{
	if (!DialogSetToUse)
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogSetToUse is null, cannot update DialogSet data"));
		DialogSet = nullptr;
		return;
	}

	if (!DialogSet)
	{
		DialogSet = NewObject<UDialogSet>(this, NAME_None, RF_Transactional);
		DialogSet->OwningDialogGraph = GetOwningDialogGraph();
	}

	DialogSet->Name = DialogSetToUse->Name;
	DialogSet->PlacementTransform = DialogSetToUse->PlacementTransform;

	//Populate Dialog Definition Slots
	{
		const int32 SourceCount = DialogSetToUse->Slots.Num();
		DialogSet->Slots.SetNum(SourceCount);
		for (int32 idx = 0; idx < SourceCount; ++idx)
		{
			UDialogSequenceSlot* Source = DialogSetToUse->Slots[idx];
			if (!Source)
			{
				DialogSet->Slots[idx] = nullptr;
				continue;
			}

			UDialogSequenceSlot* Target = DialogSet->Slots[idx];
			if (!Target)
			{
				Target = NewObject<UDialogSequenceSlot>(DialogSet, NAME_None, RF_Transactional);
			}

			Target->OwningDialogGraph = GetOwningDialogGraph();
			Target->ActorTag = Source->ActorTag;
			Target->DialogDefinition = Source->DialogDefinition;
			Target->SlotPlacement = Source->SlotPlacement;

			DialogSet->Slots[idx] = Target;
		}
	}

	//Populate Camera Slots
	/*{
		const int32 SourceCount = DialogSetToUse->CameraSlots.Num();
		DialogSet->CameraSlots.SetNum(SourceCount);
		for (int32 idx = 0; idx < SourceCount; ++idx)
		{
			UDialogSequenceSlot* Source = DialogSetToUse->CameraSlots[idx];
			if (!Source)
			{
				DialogSet->CameraSlots[idx] = nullptr;
				continue;
			}

			UDialogSequenceSlot* Target = DialogSet->CameraSlots[idx];
			if (!Target)
			{
				Target = NewObject<UDialogSequenceSlot>(DialogSet, NAME_None, RF_Transactional);
			}

			Target->OwningDialogGraph = GetOwningDialogGraph();
			Target->ActorTag = Source->ActorTag;
			Target->DialogDefinition = Source->DialogDefinition;
			Target->SlotPlacement = Source->SlotPlacement;

			DialogSet->CameraSlots[idx] = Target;
		}
	}*/
}


#if WITH_EDITOR

FText UDialogBuilderNode_DialogSequence::GetNodeTitle() const
{
    return FText::FromString("Dialog Sequence");
}

void UDialogBuilderNode_DialogSequence::SetNodeTitle(const FText& NewTitle)
{
    ID = FName(NewTitle.ToString());
}

FText UDialogBuilderNode_DialogSequence::GetNodeDescription() const
{
    return FText::FromString("Desc");
}


void UDialogBuilderNode_DialogSequence::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UDialogBuilderNode_DialogSequence::PostLoad()
{
	Super::PostLoad();
}

#endif

void UDialogBuilderNode_DialogSequence::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
}
#undef LOCTEXT_NAMESPACE

