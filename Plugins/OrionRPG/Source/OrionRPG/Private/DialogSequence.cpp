// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "DialogSequence.h"
#include "MovieScene.h"
#include "MovieSceneDialogSection.h"
#include "MovieSceneDialogTrack.h"
#include "Tracks/MovieSceneCameraCutTrack.h"
#include "Tracks/MovieSceneAudioTrack.h"
#include "Tracks/MovieSceneEventTrack.h"
#include "Tracks/MovieSceneMaterialParameterCollectionTrack.h"
#include "Tracks/MovieSceneSkeletalAnimationTrack.h"
#include "Tracks/MovieSceneTimeWarpTrack.h"

UDialogSequence::UDialogSequence(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MaxFrame = 0;
	
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
	UMovieSceneDialogTrack* NewDialogTrack = MovieScene->AddTrack<UMovieSceneDialogTrack>();
	NewDialogTrack->SetDisplayName(FText::FromString("Dialog"));
	return NewDialogTrack;
}

const UMovieSceneDialogTrack* UDialogSequence::FindDialogTrack() const
{
	if (!MovieScene)
	{
		return nullptr;
	}

	return MovieScene->FindTrack<UMovieSceneDialogTrack>();
}

void UDialogSequence::RefreshSequence()
{
	DialogTrack = GetOrCreateDialogTrack();
	if (!MovieScene || !DialogTrack)
	{
		return;
	}

	bool bHasSections = false;
	FFrameNumber MinFrame = 0;
	MaxFrame = 0;

	for (UMovieSceneSection* Section : DialogTrack->GetAllSections())
	{
		if (!Section)
		{
			continue;
		}

		const TRange<FFrameNumber> SectionRange = Section->GetRange();
		if (!SectionRange.HasLowerBound() || !SectionRange.HasUpperBound())
		{
			continue;
		}

		const FFrameNumber SectionStart = SectionRange.GetLowerBoundValue();
		const FFrameNumber SectionEnd = SectionRange.GetUpperBoundValue();

		if (!bHasSections)
		{
			MinFrame = SectionStart;
			MaxFrame = SectionEnd;
			bHasSections = true;
		}
		else
		{
			MinFrame = SectionStart < MinFrame ? SectionStart : MinFrame;
			MaxFrame = MaxFrame < SectionEnd ? SectionEnd : MaxFrame;
		}
	}


	if (bHasSections)
	{
		const TRange<FFrameNumber> PlaybackRange(0, MaxFrame);
		MovieScene->SetPlaybackRange(PlaybackRange);

		if (UMovieSceneTrack* CameraCutTrack = MovieScene->GetCameraCutTrack())
		{
			for (UMovieSceneSection* Section : CameraCutTrack->GetAllSections())
			{
				if (!Section) continue;
				
				const TRange<FFrameNumber> SectionRange = Section->GetRange();
				if (!SectionRange.HasUpperBound() || SectionRange.GetUpperBoundValue() != MaxFrame)
				{
					Section->SetRange(PlaybackRange);
				}
			}
		}
	}

	
	MovieScene->SetPlaybackRangeLocked(true);

}

#if WITH_EDITOR
UDialogSequence* UDialogSequence::GetNullDialogSequence()
{
	static UDialogSequence* NullSequence = nullptr;

	if (!NullSequence)
	{
		NullSequence = NewObject<UDialogSequence>(GetTransientPackage(), NAME_None);
		NullSequence->AddToRoot();
		NullSequence->MovieScene = NewObject<UMovieScene>(NullSequence, FName("No Animation"));
		NullSequence->MovieScene->AddToRoot();

		NullSequence->MovieScene->SetDisplayRate(FFrameRate(20, 1));
	}

	return NullSequence;
}
#endif

