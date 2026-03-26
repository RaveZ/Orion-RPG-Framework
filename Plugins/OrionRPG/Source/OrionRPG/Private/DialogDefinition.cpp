// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogDefinition.h"
#include "Misc/Guid.h"


#define LOCTEXT_NAMESPACE "DialogDefinition"

UDialogDefinition::UDialogDefinition()
{
}


UDialogParticipant::UDialogParticipant()
{
	ParticipantInfo.ParticipantSetup = EParticipantSetup::E_SpawnParticipant;
}

UDialogProp::UDialogProp()
{
}

UDialogCamera::UDialogCamera()
{
}

#undef LOCTEXT_NAMESPACE

