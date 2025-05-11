// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "UObject/NoExportTypes.h"
#include "QuestBuilderGraph.h"
#include "DialogBuilderEdNode_Edge.generated.h"


class UQuestBuilderNode;
class UQuestBuilderEdge;
class UDialogBuilderEdNode;
class UEdGraph;

UCLASS()
class DIALOG_SYSTEM_EDITOR_API UDialogBuilderEdNode_Edge : public UEdGraphNode
{
	GENERATED_BODY()
public:
	UDialogBuilderEdNode_Edge();

	UPROPERTY()
		TObjectPtr<UEdGraph> Graph;

	UPROPERTY(VisibleAnywhere, Instanced, Category = "QuestBuilderGraph")
		TObjectPtr<UQuestBuilderEdge> QuestSystemGraphEdge;

	void SetEdge(UQuestBuilderEdge* Edge);

	virtual void AllocateDefaultPins() override;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;

	virtual void PrepareForCopying() override;

	virtual UEdGraphPin* GetInputPin() const { return Pins[0]; }
	virtual UEdGraphPin* GetOutputPin() const { return Pins[1]; }

	void CreateConnections(UDialogBuilderEdNode* Start, UDialogBuilderEdNode* End);

	UDialogBuilderEdNode* GetStartNode();
	UDialogBuilderEdNode* GetEndNode();
};
