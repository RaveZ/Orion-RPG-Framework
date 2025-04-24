// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "Setting_QuestEditor.h"
#include "UObject/ConstructorHelpers.h"
#include "QuestNodeWidget.h"

UQuestEditorSettings::UQuestEditorSettings()
{
	auto QuestNodeUserWidgetFinder = ConstructorHelpers::FClassFinder<UQuestNodeWidget>(TEXT("WidgetBlueprint'/Quest_System_Editor/Widgets/Editor/WBP_DefaultQuestNodeWidget.WBP_DefaultQuestNodeWidget_C'"));
	if (QuestNodeUserWidgetFinder.Succeeded())
	{
		DefaultQuestNodeWidgetClass = QuestNodeUserWidgetFinder.Class;
	}
}

UQuestEditorSettings::~UQuestEditorSettings()
{
}
