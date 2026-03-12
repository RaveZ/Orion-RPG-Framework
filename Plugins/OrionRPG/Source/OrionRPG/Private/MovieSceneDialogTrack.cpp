// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "MovieSceneDialogTrack.h"
#include "MovieSceneDialogSection.h"
#include "DialogData.h"

UMovieSceneDialogTrack::UMovieSceneDialogTrack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TrackTint = FColor(52, 152, 219);
}

UMovieSceneDialogSection* UMovieSceneDialogTrack::AddDialogSection(
	const FDialogLineData& InDialogLineData,
	const FGameplayTag& InSpeakerParticipantTag,
	const FGameplayTag& InListenerParticipantTag,
	FFrameNumber InStartFrame,
	int32 InDurationFrames)
{
	UMovieSceneDialogSection* NewSection = Cast<UMovieSceneDialogSection>(CreateNewSection());
	if (!NewSection)
	{
		return nullptr;
	}

	NewSection->SpeakerParticipantTag = InSpeakerParticipantTag;
	NewSection->ListenerParticipantTag = InListenerParticipantTag;
	NewSection->DialogLineData = InDialogLineData;

	const int32 ClampedDurationFrames = FMath::Max(1, InDurationFrames);
	NewSection->SetRange(TRange<FFrameNumber>(
		TRangeBound<FFrameNumber>::Inclusive(InStartFrame),
		TRangeBound<FFrameNumber>::Exclusive(InStartFrame + ClampedDurationFrames)));

	AddSection(*NewSection);

	return NewSection;
}

bool UMovieSceneDialogTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass == UMovieSceneDialogSection::StaticClass();
}

UMovieSceneSection* UMovieSceneDialogTrack::CreateNewSection()
{
	return NewObject<UMovieSceneDialogSection>(this, NAME_None, RF_Transactional);
}

const TArray<UMovieSceneSection*>& UMovieSceneDialogTrack::GetAllSections() const
{
	return Sections;
}

void UMovieSceneDialogTrack::RemoveAllAnimationData()
{
	Sections.Reset();
}

bool UMovieSceneDialogTrack::HasSection(const UMovieSceneSection& Section) const
{
	return Sections.Contains(&Section);
}

void UMovieSceneDialogTrack::AddSection(UMovieSceneSection& Section)
{
	Sections.Add(&Section);
}

void UMovieSceneDialogTrack::RemoveSection(UMovieSceneSection& Section)
{
	Sections.Remove(&Section);
}

bool UMovieSceneDialogTrack::IsEmpty() const
{
	return Sections.Num() == 0;
}

bool UMovieSceneDialogTrack::SupportsMultipleRows() const
{
	return false;
}

#if WITH_EDITORONLY_DATA
FText UMovieSceneDialogTrack::GetDefaultDisplayName() const
{
	return NSLOCTEXT("OrionRPG", "MovieSceneDialogTrackName", "Dialog");
}
#endif