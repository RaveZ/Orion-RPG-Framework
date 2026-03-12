// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogBuilderNode.h"
#include "DialogData.h"
#include <MovieSceneSequencePlayer.h>
#include <CineCameraSettings.h>
#include "LevelSequencePlayer.h"
#include "DialogBuilderNode_DialogSequence.generated.h"


class UDialogSequence;
UCLASS(Blueprintable, BlueprintType, AutoExpandCategories = "Shot Override")
class ORIONRPG_API UDialogBuilderNode_DialogSequence : public UDialogBuilderNode
{
	GENERATED_BODY()

public:
	UDialogBuilderNode_DialogSequence();
	
public:
	virtual void BeginNode() override;
	
	virtual void EvaluateNextNode() override;

	void EnsureSequenceCreated();


public:
	virtual void Serialize(FArchive& Ar) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sequence")
	TObjectPtr<UDialogSequence> DialogSequence = nullptr;

#if WITH_EDITORONLY_DATA
	TWeakPtr<ISequencer> Sequencer;
#endif
public:
#if WITH_EDITOR

	//Node Setting
	virtual FText GetNodeTitle() const override;
	virtual void SetNodeTitle(const FText& NewTitle);
	virtual FText GetNodeDescription() const override;


	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostLoad() override;
	//~ End UObject Interface
#endif
};
