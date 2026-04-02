// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogDefinition.h"
#include "CineCameraActor.h"
#include "Misc/Guid.h"


#define LOCTEXT_NAMESPACE "DialogDefinition"

UDialogDefinition::UDialogDefinition()
{
	GetOrCreateID();
}

FGuid UDialogDefinition::GetOrCreateID()
{
	if(!ID.IsValid())
	{
		ID = FGuid::NewGuid();
	}

	return ID;
}

UDialogPivot::UDialogPivot()
{
	DisplayName = FText::FromString("Dialog Pivot");
	ActorClass = AActor::StaticClass();	
}


UDialogParticipant::UDialogParticipant()
{
	DisplayName = FText::FromString("Participant");
	Tag = FGameplayTag::RequestGameplayTag(FName("Dialog.Participant"));
}

UDialogProp::UDialogProp()
{
	DisplayName = FText::FromString("Prop");
}

UDialogCamera::UDialogCamera()
{
	DisplayName = FText::FromString("Camera");
	CameraClass = ACineCameraActor::StaticClass();
}

#undef LOCTEXT_NAMESPACE

