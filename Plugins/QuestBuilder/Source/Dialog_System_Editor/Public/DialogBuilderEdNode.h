// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "AIGraphTypes.h"
#include "UObject/ObjectMacros.h"
#include "UObject/NoExportTypes.h"
#include "DialogBuilderEdGraph.h"
#include "DialogBuilderGraph.h"
#include "DialogBuilderEdNode.generated.h"

class UEdNode_DialogSystemEdge;
class UDialogBuilderEdGraph;
class SGraphNode_DialogBuilderNode;

UCLASS()
class DIALOG_SYSTEM_EDITOR_API UDialogBuilderEdNode : public UEdGraphNode
{
	GENERATED_BODY()
public:
	UDialogBuilderEdNode();
	virtual ~UDialogBuilderEdNode();

	/** instance class */
	UPROPERTY()
	struct FGraphNodeClassData ClassData;

	UPROPERTY()
	TObjectPtr<UObject> NodeInstance;
	
	UDialogBuilderEdGraph* GetDialogBuilderEdGraph();

	SGraphNode_DialogBuilderNode* SGraphNode;

	//UedGraphnode interface
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool CanDuplicateNode() const override;
	virtual void PostPlacedNewNode() override;
	virtual bool CanUserDeleteNode() const override;
	virtual void PrepareForCopying() override;
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;


	virtual FText GetDescription() const;
	virtual FText GetEventsText() const;
	virtual FText GetConditionsText() const;

	virtual FLinearColor GetBackgroundColor() const;
	virtual UEdGraphPin* GetInputPin() const;
	virtual UEdGraphPin* GetOutputPin() const;


	/** Gets a list of actions that can be done to this particular node */
	virtual void GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;

	/** initialize instance object  */
	virtual void InitializeInstance();

	/** Check if node instance uses blueprint for its implementation */
	bool UsesBlueprint() const;

#if WITH_EDITOR
	virtual void PostEditUndo() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/** if set, all modifications (including delete/cut) are disabled */
	UPROPERTY()
	uint32 bIsReadOnly : 1;
};
