// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "DialogBuilderSetting.h"
#include "UObject/ConstructorHelpers.h"

UDialogBuilderSetting::UDialogBuilderSetting()
{
	SpeakerNodeColor = FLinearColor(0.15f, 0.15f, 0.15f);
	OptionSelectorNodeColor = FLinearColor(.063f, .096f, .1875f);
	EndDialogNodeColor = FLinearColor(.34f, .024f, .024f);
	PlayerOptionNodeColor = FLinearColor(.123f, .274f, .34f);
	SubNodeColor = FLinearColor(0.05f, 0.05f, 0.05f, 0.7f);
	RootNodeColor = FLinearColor(0.5f, 0.5f, 0.5f, 0.2f);
	bEdgeEnabled = false;            
	bCanRenameNode = true;
	bCanBeCyclical = false;
}


UDialogBuilderSetting::~UDialogBuilderSetting()
{
}
