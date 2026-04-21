// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "DialogStage.h"
#include "DialogBuilderGraph.h"
#include "DialogBuilderNode_DialogSequence.h"
#include "DialogDefinition.h"
#include "CineCameraActor.h"
#include "Misc/Guid.h"

#define LOCTEXT_NAMESPACE "DialogStage"

UDialogSequenceSlot::UDialogSequenceSlot()
{
}

FGameplayTag UDialogSequenceSlot::GetActorTag() const
{
	if (DialogDefinition)
	{
		return DialogDefinition->Tag;
	}
	return FGameplayTag::EmptyTag;
}

void UDialogSequenceSlot::SetDialogDefinition(UDialogDefinition* InDialogDefinition)
{
	Modify();
	DialogDefinition = InDialogDefinition;

	if(OwningDialogGraph)
	{
		TArray<UDialogBuilderNode_DialogSequence*> OutNodes;
		OwningDialogGraph->GetNodesOfType<UDialogBuilderNode_DialogSequence>(OutNodes);
		for (UDialogBuilderNode_DialogSequence* SequenceNode : OutNodes)
		{
			if (SequenceNode)
			{
				SequenceNode->Modify();
				SequenceNode->UpdateDialogStageData();
			}
		}
	}
}

#if WITH_EDITOR


void UDialogSequenceSlot::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	
}
#endif

UDialogStage::UDialogStage()
{
}

#if WITH_EDITOR
void UDialogStage::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

#undef LOCTEXT_NAMESPACE

