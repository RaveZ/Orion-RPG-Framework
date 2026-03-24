// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "DialogSequence.h"
#include "MovieScene.h"
#include "MovieSceneDialogSection.h"
#include "MovieSceneDialogTrack.h"
#include "Tracks/MovieSceneAudioTrack.h"
#include "Tracks/MovieSceneEventTrack.h"
#include "Tracks/MovieSceneMaterialParameterCollectionTrack.h"
#include "Tracks/MovieSceneSkeletalAnimationTrack.h"
#include "Tracks/MovieSceneTimeWarpTrack.h"

UDialogSequence::UDialogSequence(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UMovieSceneDialogTrack* UDialogSequence::GetOrCreateDialogTrack()
{
	if (!MovieScene)
	{
		return nullptr;
	}

	if (UMovieSceneDialogTrack* ExistingTrack = MovieScene->FindTrack<UMovieSceneDialogTrack>())
	{
		return ExistingTrack;
	}

	Modify();
	MovieScene->Modify();

	return MovieScene->AddTrack<UMovieSceneDialogTrack>();
}

const UMovieSceneDialogTrack* UDialogSequence::FindDialogTrack() const
{
	if (!MovieScene)
	{
		return nullptr;
	}

	return MovieScene->FindTrack<UMovieSceneDialogTrack>();
}


