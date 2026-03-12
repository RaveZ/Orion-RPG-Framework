// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "MovieSceneDialogSection.h"

UMovieSceneDialogSection::UMovieSceneDialogSection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsActive(true);
	SetIsLocked(false);
}