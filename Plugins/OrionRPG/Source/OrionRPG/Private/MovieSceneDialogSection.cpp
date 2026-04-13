// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "MovieSceneDialogSection.h"
#include "DialogSequence.h"
#include "DialogBuilderGraph.h"
#include "DialogDefinition.h"

#define LOCTEXT_NAMESPACE "DialogSection"

namespace
{
static bool IsParticipantInGraph(const UDialogBuilderGraph* Graph, const UDialogParticipant* Participant)
{
	if (!Graph || !Participant)
	{
		return false;
	}

	for (const TObjectPtr<UDialogParticipant>& Entry : Graph->ParticipantDefinitions)
	{
		if (Entry.Get() == Participant)
		{
			return true;
		}
	}

	return false;
}
}

UMovieSceneDialogSection::UMovieSceneDialogSection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsActive(true);
	SetIsLocked(false);
}

UDialogSequence* UMovieSceneDialogSection::GetOwningDialogSequence() const
{
	return GetTypedOuter<UDialogSequence>();
}

UDialogBuilderGraph* UMovieSceneDialogSection::GetOwningDialogGraph() const
{
	if (UDialogSequence* DialogSequence = GetOwningDialogSequence())
	{
		return DialogSequence->OwningDialogGraph;
	}
	return nullptr;
}

void UMovieSceneDialogSection::RefreshDialogContent()
{
	UDialogBuilderGraph* DialogGraph = GetOwningDialogGraph();

	// Speaker reference validity
	if (!IsParticipantInGraph(DialogGraph, SpeakerParticipantDefinition))
	{
		SpeakerParticipantDefinition = nullptr;
	}

	// Listener reference validity
	if (!IsParticipantInGraph(DialogGraph, ListenerParticipantDefinition))
	{
		ListenerParticipantDefinition = nullptr;
	}

	//DialogLine.SpeakerName = DialogGraph ? DialogGraph->GetParticipantName(DialogLine.SpeakerTag) : FText();
	DialogLine.SpeakerName = SpeakerParticipantDefinition ? SpeakerParticipantDefinition->DisplayName : FText();
}

#if WITH_EDITOR
void UMovieSceneDialogSection::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	RefreshDialogContent();
}
#endif

#undef LOCTEXT_NAMESPACE