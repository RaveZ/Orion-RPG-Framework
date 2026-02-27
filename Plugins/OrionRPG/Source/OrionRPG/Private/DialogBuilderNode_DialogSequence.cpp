// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderNode_DialogSequence.h"
#include "DialogBuilderNode_PlayerChoice.h"
#include "Decorator/OrionDecorator.h"
#include "DialogBuilderGraph.h"
#include "DialogBuilderSetting.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Animation/AnimMontage.h"
#include "Sound/SoundBase.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "DialogComponent.h"


#define LOCTEXT_NAMESPACE "DialogSystemGraphNode"


UDialogBuilderNode_DialogSequence::UDialogBuilderNode_DialogSequence()
{
}

void UDialogBuilderNode_DialogSequence::BeginNode()
{
	Super::BeginNode();
	
}

void UDialogBuilderNode_DialogSequence::EvaluateNextNode()
{
	
	Super::EvaluateNextNode();
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

