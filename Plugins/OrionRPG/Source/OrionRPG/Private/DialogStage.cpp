// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "DialogStage.h"
#include "DialogBuilderGraph.h"
#include "DialogDefinition.h"
#include "CineCameraActor.h"
#include "Misc/Guid.h"

#define LOCTEXT_NAMESPACE "DialogStage"

UDialogSequenceSlot::UDialogSequenceSlot()
{
}

#if WITH_EDITOR
void UDialogSequenceSlot::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Keep ActorTag synchronized from DialogDefinition reference
	if (UDialogParticipant* Participant = Cast<UDialogParticipant>(DialogDefinition))
	{
		ActorTag = Participant->Tag;
	}
	else
	{
		ActorTag = FGameplayTag();
	}
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

