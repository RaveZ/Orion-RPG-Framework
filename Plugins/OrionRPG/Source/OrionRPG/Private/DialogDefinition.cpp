// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogDefinition.h"


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


#undef LOCTEXT_NAMESPACE

