// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Setting_QuestEditor.generated.h"



UCLASS(config = Engine, defaultconfig)
class QUEST_SYSTEM_EDITOR_API UQuestEditorSettings : public UObject
{
	GENERATED_BODY()

	UQuestEditorSettings();
	virtual ~UQuestEditorSettings();

public:
	UPROPERTY(EditAnywhere, config, Category = "Graph Defaults", noclear, meta = (MetaClass = "/Script/Quest_System_Editor.QuestNodeWidget"))
	TSoftClassPtr<class UQuestNodeWidget> DefaultQuestNodeWidgetClass;
};
