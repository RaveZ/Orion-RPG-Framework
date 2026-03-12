// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LevelSequence.h"
#include "DialogSequence.generated.h"

class UMovieSceneDialogSection;
class UMovieSceneDialogTrack;
struct FDialogLineData;
struct FGameplayTag;

UCLASS(BlueprintType)
class ORIONRPG_API UDialogSequence : public ULevelSequence
{
	GENERATED_BODY()

public:
	UDialogSequence(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UMovieSceneDialogTrack* GetOrCreateDialogTrack();
	const UMovieSceneDialogTrack* FindDialogTrack() const;

	UMovieSceneDialogSection* AddDialogSection(
		const FDialogLineData& InDialogLineData,
		const FGameplayTag& InSpeakerParticipantTag,
		const FGameplayTag& InListenerParticipantTag,
		FFrameNumber InStartFrame,
		int32 InDurationFrames = 1);
};