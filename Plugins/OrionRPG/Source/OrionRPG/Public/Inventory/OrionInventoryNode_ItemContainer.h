// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OrionInventoryNode.h"
#include "OrionInventoryNode_ItemContainer.generated.h"

UCLASS(Blueprintable, BlueprintType)
class ORIONRPG_API UOrionInventoryNode_ItemContainer : public UOrionInventoryNode
{
	GENERATED_BODY()

public:
	UOrionInventoryNode_ItemContainer();

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail", meta = (ClampMin = "5", ClampMax = "10", UIMin = "5", UIMax = "10"))
    int32 GridCountX;
    

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail")
    int32 GridCountY;

public:
#if WITH_EDITOR

	//Node Setting
	virtual FText GetNodeTitle() const override;
	virtual void SetNodeTitle(const FText& NewTitle);
	virtual FText GetNodeDescription() const override;
#endif
};
