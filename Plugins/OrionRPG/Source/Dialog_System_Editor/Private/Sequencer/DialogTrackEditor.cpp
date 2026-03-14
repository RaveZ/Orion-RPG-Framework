// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Sequencer/DialogTrackEditor.h"
#include "Sequencer/DialogSection.h"
#include "MovieSceneDialogSection.h"
#include "MovieSceneDialogTrack.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "EditorStyleSet.h"
#include "SequencerUtilities.h"
#include "IDetailsView.h"
//#include "ModuleManager.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"

//#include "MovieSceneAudioTrack.h"
#include "Tracks/MovieSceneAudioTrack.h"
#include "Sound/SoundBase.h"

TSharedRef<ISequencerTrackEditor> FDialogTrackEditor::CreateTrackEditor(TSharedRef<ISequencer> InSequencer)
{
	return MakeShared<FDialogTrackEditor>(InSequencer);
}

FDialogTrackEditor::FDialogTrackEditor(TSharedRef<ISequencer> InSequencer)
	: FMovieSceneTrackEditor(InSequencer)
{
}

bool FDialogTrackEditor::SupportsType(TSubclassOf<UMovieSceneTrack> Type) const
{
	return Type == UMovieSceneDialogTrack::StaticClass();
}

TSharedRef<ISequencerSection> FDialogTrackEditor::MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding)
{
	UMovieSceneDialogSection* DialogSection = Cast<UMovieSceneDialogSection>(&SectionObject);
	check(SupportsType(SectionObject.GetOuter()->GetClass()) && DialogSection != nullptr);

	return MakeShareable(new FDialogSection(*DialogSection));
}