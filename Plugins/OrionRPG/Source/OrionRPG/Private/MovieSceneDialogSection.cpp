// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "MovieSceneDialogSection.h"
#include "DialogSequence.h"
#include "DialogBuilderGraph.h"

#define LOCTEXT_NAMESPACE "DialogSection"
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

void UMovieSceneDialogSection::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName DialogLineProperty = FName("DialogLine");
	if (DialogLineProperty == GET_MEMBER_NAME_CHECKED(UMovieSceneDialogSection, DialogLine))
	{
		UDialogBuilderGraph* DialogGraph = GetOwningDialogGraph();
		DialogLine.SpeakerName = DialogGraph ? DialogGraph->GetParticipantName(DialogLine.SpeakerTag) : FText();
	}
}

#undef LOCTEXT_NAMESPACE