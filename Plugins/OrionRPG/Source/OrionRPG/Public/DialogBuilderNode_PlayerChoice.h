// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderNode_PlayerChoice.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnPlayerChoiceDataChanged);

UCLASS(BlueprintType)
class ORIONRPG_API UDialogBuilderNode_PlayerChoice : public UDialogBuilderNode
{
	GENERATED_BODY()
public:
	UDialogBuilderNode_PlayerChoice();
	virtual void BeginNode() override;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Detail")
	FText ChoiceText;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Detail")
	TArray<FText> ChoiceList;

	FOnPlayerChoiceDataChanged OnPlayerChoiceDataChanged;

#if WITH_EDITOR
	virtual FText GetNodeTitle() const override;
	virtual void SetNodeTitle(const FText& NewTitle);
	virtual FText GetNodeDescription() const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
};
