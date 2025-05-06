// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "QuestSystemGraph.generated.h"


class UQuestSystem;
class UQuestComponent;
class UQuestSystemGraphNode;
class UQuestSystemGraphEdge;
class AController;
class APawn;
class UQuestBuilderEdGraph;

UCLASS(Blueprintable)
class QUEST_SYSTEM_RUNTIME_API UQuestSystemGraph : public UObject
{
	GENERATED_BODY()
public:
	UQuestSystemGraph();
	virtual ~UQuestSystemGraph();

	/**This data will be displayed on the category for your quest log*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "QuestSystemGraph")
		FString CategoryDisplayName;

	UPROPERTY(BlueprintReadOnly, BlueprintReadOnly, Category = "QuestSystemGraph")
		FGameplayTagContainer GraphTags;

	/** Set of QuestSystem linked with Quest Graph Pages */
	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraph")
		TArray< TObjectPtr<UQuestSystem>> QuestSystems;

	UPROPERTY(BlueprintReadOnly, BlueprintReadOnly, Category = "QuestSystemGraph")
		bool bEdgeEnabled;

	UPROPERTY(BlueprintReadWrite, Category = "QuestSystemGraph")
		TObjectPtr<AController> OwningController;

	UPROPERTY(BlueprintReadWrite, Category = "QuestSystemGraph")
		TObjectPtr < UQuestComponent> QuestComponent;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraph")
		TObjectPtr<APawn> OwningPawn;


#if WITH_EDITORONLY_DATA
	/** Set of quest-graph pages */
	UPROPERTY()
	TArray<TObjectPtr<UEdGraph>> QuestGraphPages;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraph")
		bool bCanRenameNode;

	UPROPERTY(BlueprintReadOnly, Category = "QuestSystemGraph")
		bool bCanBeCyclical;

	/** Set of documents that were being edited in this blueprint, so we can open them right away */
	UPROPERTY()
	TArray<struct FEditedDocumentInfo> LastEditedDocuments;

	/** Whether or not this blueprint is newly created, and hasn't been opened in an editor yet */
	UPROPERTY(/*transient*/ BlueprintReadOnly, Category = "QuestSystemGraph")
	bool bIsNewlyCreated = true;


	/** Get all graphs in this QuestEditor */
	void GetAllGraphs(TArray<UEdGraph*>& Graphs) const;
#endif

};

