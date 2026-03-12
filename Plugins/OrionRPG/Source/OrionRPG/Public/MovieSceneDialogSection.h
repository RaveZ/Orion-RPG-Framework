// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MovieSceneSection.h"
#include "DialogData.h"
#include "MovieSceneDialogSection.generated.h"

UCLASS(BlueprintType)
class ORIONRPG_API UMovieSceneDialogSection : public UMovieSceneSection
{
	GENERATED_BODY()

public:
	UMovieSceneDialogSection(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	FGameplayTag SpeakerParticipantTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	FGameplayTag ListenerParticipantTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	FDialogLineData DialogLineData;
};