// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "Setting_QuestEditor.h"
#include "UObject/ConstructorHelpers.h"

UQuestEditorSettings::UQuestEditorSettings()
{
	FailedNodeColor = FLinearColor(0.38f, 0.f, 0.f);
	SuccessNodeColor = FLinearColor(0.f, 0.38f, 0.f);
	TaskNodeColor = FLinearColor(0.65f, 0.28f, 0.f);
	StateNodeColor = FLinearColor(0.1f, 0.1f, 0.1f);
	PersistentTasksNodeColor = FLinearColor(0.1f, 0.1f, 0.1f);
	RootNodeColor = FLinearColor(0.1f, 0.1f, 0.1f);
}

UQuestEditorSettings::~UQuestEditorSettings()
{
}
