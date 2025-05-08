// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "QuestBuilderGraph.generated.h"


class UQuest;
class UQuestComponent;
class UQuestBuilderNode;
class UQuestBuilderEdge;
class AController;
class APawn;
class UQuestBuilderEdGraph;

UCLASS(Blueprintable)
class QUEST_SYSTEM_RUNTIME_API UQuestBuilderGraph : public UObject
{
	GENERATED_BODY()
public:
	UQuestBuilderGraph();
	virtual ~UQuestBuilderGraph();

	/**This data will be displayed on the category for your quest log*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "QuestBuilderGraph")
		FString CategoryDisplayName;

	UPROPERTY(BlueprintReadOnly, BlueprintReadOnly, Category = "QuestBuilderGraph")
		FGameplayTagContainer GraphTags;

	/** Set of Quest linked with Quest Graph Pages */
	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderGraph")
		TArray< TObjectPtr<UQuest>> QuestList;

	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderGraph")
		TMap<FName, TObjectPtr<UQuest>> QuestMap;

	UPROPERTY(BlueprintReadOnly, BlueprintReadOnly, Category = "QuestBuilderGraph")
		bool bEdgeEnabled;

	UPROPERTY(BlueprintReadWrite, Category = "QuestBuilderGraph")
		TObjectPtr<AController> OwningController;

	UPROPERTY(BlueprintReadWrite, Category = "QuestBuilderGraph")
		TObjectPtr < UQuestComponent> QuestComponent;

	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderGraph")
		TObjectPtr<APawn> OwningPawn;


#if WITH_EDITORONLY_DATA
	/** Set of quest-graph pages */
	UPROPERTY()
	TArray<TObjectPtr<UEdGraph>> QuestGraphPages;

	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderGraph")
		bool bCanRenameNode;

	UPROPERTY(BlueprintReadOnly, Category = "QuestBuilderGraph")
		bool bCanBeCyclical;

	/** Set of documents that were being edited in this blueprint, so we can open them right away */
	UPROPERTY()
	TArray<struct FEditedDocumentInfo> LastEditedDocuments;

	/** Whether or not this blueprint is newly created, and hasn't been opened in an editor yet */
	UPROPERTY(/*transient*/ BlueprintReadOnly, Category = "QuestBuilderGraph")
	bool bIsNewlyCreated = true;


	/** Get all graphs in this QuestEditor */
	void GetAllGraphs(TArray<UEdGraph*>& Graphs) const;
#endif

};

