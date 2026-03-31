// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogSet.h"
#include "DialogBuilderGraph.h"
#include "CineCameraActor.h"
#include "Misc/Guid.h"


#define LOCTEXT_NAMESPACE "DialogSet"

UDialogSequenceSlot::UDialogSequenceSlot()
{
}


#if WITH_EDITOR
void UDialogSequenceSlot::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if(OwningDialogGraph)
	{
		DialogDefinition = OwningDialogGraph->GetDialogDefinitionByTag(ActorTag);
	}
}
#endif

UDialogSet::UDialogSet()
{
}

#if WITH_EDITOR
void UDialogSet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	for (TObjectPtr<UDialogSequenceSlot>& Slot : Slots)
	{
		if (Slot)
		{
			Slot->OwningDialogGraph = OwningDialogGraph;
		}
	}
	
}

#endif


#undef LOCTEXT_NAMESPACE

