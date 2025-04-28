// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestNodeWidget.generated.h"

class UQuestSystemGraphNode;
class UQuestSystem;
class UVerticalBox;

UCLASS()
class QUEST_SYSTEM_EDITOR_API UQuestNodeWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	void InitializeFromNode(class UQuestSystemGraphNode* InNode, class UQuestSystem* InQuest);

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Node")
	void OnNodeInitialized(class UQuestSystemGraphNode* InNode, class UQuestSystem* InQuest);

protected:

	UPROPERTY(BlueprintReadOnly, Category = "Quest Node")
		TObjectPtr<UQuestSystemGraphNode> Node;

	UPROPERTY(BlueprintReadOnly, Category = "Quest Node")
		TObjectPtr<UQuestSystem> Quest;

public:

	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UVerticalBox> LeftPinBox;

	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UVerticalBox> RightPinBox;
};
