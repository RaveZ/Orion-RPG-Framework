// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "AIGraphTypes.h"
#include "UObject/ObjectMacros.h"
#include "UObject/NoExportTypes.h"
#include "QuestBuilderEdGraph.h"
#include "QuestBuilderGraph.h"
#include "QuestBuilderEdNode.generated.h"

class UEdNode_QuestSystemEdge;
class UQuestBuilderEdGraph;
class SGraphNode_QuestBuilderNode;

UCLASS()
class QUEST_SYSTEM_EDITOR_API UQuestBuilderEdNode : public UEdGraphNode
{
	GENERATED_BODY()
public:
	UQuestBuilderEdNode();
	virtual ~UQuestBuilderEdNode();

	/** instance class */
	UPROPERTY()
	struct FGraphNodeClassData ClassData;

	UPROPERTY()
	TObjectPtr<UObject> NodeInstance;
	
	UQuestBuilderEdGraph* GetQuestBuilderEdGraph();

	SGraphNode_QuestBuilderNode* SGraphNode;

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
