// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "QuestBuilderSetting.h"
#include "UObject/ConstructorHelpers.h"

UQuestBuilderSetting::UQuestBuilderSetting()
{
	FailedNodeColor = FLinearColor(.34f, .024f, .024f);
	SuccessNodeColor = FLinearColor(0.20f, 0.32f, 0.15f);
	ObjectiveNodeColor = FLinearColor(0.05f, 0.05f, 0.05f);
	RootNodeColor = FLinearColor(0.05f, 0.05f, 0.05f);
	bEdgeEnabled = false;
	bCanRenameNode = true;
	bCanBeCyclical = false;
}

UQuestBuilderSetting::~UQuestBuilderSetting()
{
}
