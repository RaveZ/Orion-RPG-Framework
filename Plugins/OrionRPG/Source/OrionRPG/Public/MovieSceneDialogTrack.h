// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Compilation/IMovieSceneTrackTemplateProducer.h"
#include "MovieSceneNameableTrack.h"
#include "MovieSceneDialogTrack.generated.h"

class UMovieSceneDialogSection;
class UMovieSceneSection;
class UDialogDefinition;
struct FDialogLineData;
struct FGameplayTag;

UCLASS()
class ORIONRPG_API UMovieSceneDialogTrack
	: public UMovieSceneNameableTrack
	, public IMovieSceneTrackTemplateProducer
{
	GENERATED_BODY()

public:
	UMovieSceneDialogTrack(const FObjectInitializer& ObjectInitializer);

	UMovieSceneDialogSection* AddDialogSection(
		const FDialogLineData& InDialogLineData,
		const FGameplayTag& InSpeakerParticipantTag,
		const FGameplayTag& InListenerParticipantTag,
		FFrameNumber InStartFrame,
		int32 InDurationFrames = 1);

	/**Which dialog definition this track belong to*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog")
	TObjectPtr<class UDialogDefinition> DialogDefinition;

public:
	virtual bool SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const override;
	virtual UMovieSceneSection* CreateNewSection() override;
	virtual const TArray<UMovieSceneSection*>& GetAllSections() const override;
	virtual void RemoveAllAnimationData() override;
	virtual bool HasSection(const UMovieSceneSection& Section) const override;
	virtual void AddSection(UMovieSceneSection& Section) override;
	virtual void RemoveSection(UMovieSceneSection& Section) override;
	virtual bool IsEmpty() const override;
	virtual bool SupportsMultipleRows() const override;

	virtual FMovieSceneEvalTemplatePtr CreateTemplateForSection(const UMovieSceneSection& InSection) const override;

#if WITH_EDITORONLY_DATA
	virtual FText GetDefaultDisplayName() const override;
#endif

private:
	UPROPERTY()
	TArray<TObjectPtr<UMovieSceneSection>> Sections;
};