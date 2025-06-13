// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderSetting.h"
#include "UObject/ConstructorHelpers.h"

UQuestBuilderSetting::UQuestBuilderSetting()
{
	//Graph Setting
	FailedNodeColor = FLinearColor(.34f, .024f, .024f);
	SuccessNodeColor = FLinearColor(0.20f, 0.32f, 0.15f);
	ObjectiveNodeColor = FLinearColor(0.15f, 0.15f, 0.15f);
	CheckpointNodeColor = FLinearColor(.05f, .05f, .34f);
	SubNodeColor = FLinearColor(0.05f, 0.05f, 0.05f, 0.7f);
	RootNodeColor = FLinearColor(0.5f, 0.5f, 0.5f, 0.2f);
	bEdgeEnabled = false;
	bCanRenameNode = true;
	bCanBeCyclical = false;

	//Runtime Config
	DefaultQuestCategory = FText::FromString("Main Quest");
}

UQuestBuilderSetting::~UQuestBuilderSetting()
{
}
