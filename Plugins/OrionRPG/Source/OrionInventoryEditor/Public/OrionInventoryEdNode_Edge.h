// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "UObject/NoExportTypes.h"
#include "DialogBuilderGraph.h"
#include "OrionInventoryEdNode_Edge.generated.h"


class UDialogBuilderNode;
class UDialogBuilderEdge;
class UOrionInventoryEdNode;
class UEdGraph;

UCLASS()
class ORIONINVENTORYEDITOR_API UOrionInventoryEdNode_Edge : public UEdGraphNode
{
	GENERATED_BODY()
public:
	UOrionInventoryEdNode_Edge();

	UPROPERTY()
		TObjectPtr<UEdGraph> Graph;

	UPROPERTY(VisibleAnywhere, Instanced, Category = "OrionInventoryGraph")
		TObjectPtr<UDialogBuilderEdge> InventorySystemGraphEdge;

	void SetEdge(UDialogBuilderEdge* Edge);

	virtual void AllocateDefaultPins() override;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;

	virtual void PrepareForCopying() override;

	virtual UEdGraphPin* GetInputPin() const { return Pins[0]; }
	virtual UEdGraphPin* GetOutputPin() const { return Pins[1]; }

	void CreateConnections(UOrionInventoryEdNode* Start, UOrionInventoryEdNode* End);

	UOrionInventoryEdNode* GetStartNode();
	UOrionInventoryEdNode* GetEndNode();
};
