// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OrionInventorySetting.generated.h"

class UOrionActorViewer;

UCLASS(config = Engine, defaultconfig)
class ORIONRPG_API UOrionInventorySetting : public UObject
{
	GENERATED_BODY()

	UOrionInventorySetting();
	virtual ~UOrionInventorySetting();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Inventory Setting", noclear)
	TSubclassOf<class UOrionGridSlot> DefaultGridSlotWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Inventory Setting", noclear)
	TSubclassOf<class UOrionGridItem> DefaultGridItemWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Inventory Setting")
	TSubclassOf<class AOrionActorViewer> ItemViewerClass;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Inventory Setting")
	FVector ItemViewerLocation;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Inventory Setting")
	TSubclassOf<class AOrionActorViewer> CharacterViewerClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Inventory Setting")
	FVector CharacterViewerLocation;

	UPROPERTY(EditAnywhere, config, Category = "Graph Style")
	FLinearColor ItemContainerColor;

	UPROPERTY(BlueprintReadOnly, config, Category = "Graph Rule")
	bool bCanRenameNode;
};

