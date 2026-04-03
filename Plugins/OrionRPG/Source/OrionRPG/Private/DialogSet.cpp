// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogSet.h"
#include "DialogBuilderGraph.h"
#include "CineCameraActor.h"
#include "Misc/Guid.h"


#define LOCTEXT_NAMESPACE "DialogSet"

UDialogSequenceSlot::UDialogSequenceSlot()
{
}


#if WITH_EDITOR
void UDialogSequenceSlot::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

UDialogSet::UDialogSet()
{

}

#if WITH_EDITOR
void UDialogSet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
}

#endif


#undef LOCTEXT_NAMESPACE

