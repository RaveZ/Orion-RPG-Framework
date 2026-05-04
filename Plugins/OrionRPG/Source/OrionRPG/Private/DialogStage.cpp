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



UDialogStage* UDialogSequenceSlot::GetOwningDialogStage() const
{
	return GetTypedOuter<UDialogStage>();
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

UDialogSequenceSlot_Light::UDialogSequenceSlot_Light()
{
	LightClass = APointLight::StaticClass();
}


#if WITH_EDITOR


void UDialogSequenceSlot::UpdateActorTemplate(AActor* InActor)
{
	if (!InActor)
	{
		return;
	}

	if (ActorTemplate)
	{
		// Copy properties from the live actor back to the template
		UEngine::CopyPropertiesForUnrelatedObjects(InActor, ActorTemplate);
	}
	else
	{
		// First time initialization: Duplicate the actor into this slot
		ActorTemplate = Cast<AActor>(StaticDuplicateObject(InActor, this, NAME_None, RF_Transactional));
		if (ActorTemplate)
		{
			// Strip unwanted flags like RF_Transient since this needs to be saved
			ActorTemplate->ClearFlags(RF_Transient);
			ActorTemplate->SetFlags(RF_Public | RF_Transactional);
		}
	}

	MarkPackageDirty();
}

void UDialogSequenceSlot::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	
}
#endif

UDialogStage::UDialogStage()
{
}

UDialogBuilderNode_DialogSequence* UDialogStage::GetOwningDialogSequenceNode() const
{
	return GetTypedOuter<UDialogBuilderNode_DialogSequence>();
}


UDialogStage* UDialogStage::GetOwningDialogStage() const
{
	UDialogBuilderNode_DialogSequence* SequenceNode = GetOwningDialogSequenceNode();
	if (SequenceNode)
	{
		return SequenceNode->DialogStage;
	}
	return nullptr;
}

#if WITH_EDITOR
void UDialogStage::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);


}
#endif

#undef LOCTEXT_NAMESPACE

