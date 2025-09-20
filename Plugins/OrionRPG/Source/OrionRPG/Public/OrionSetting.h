// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OrionSetting.generated.h"


//General Setting for Orion Plugin
UCLASS(config = Engine, defaultconfig)
class ORIONRPG_API UOrionSetting : public UObject
{
	GENERATED_BODY()

	UOrionSetting();
	virtual ~UOrionSetting();

public:
	//Runtime Config

	/**Show the common UI bottom bar bound action*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "UI")
	bool ShowUIBottomBarAction;

	//Will save your last level and travel when game loaded
	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Save")
	bool bSaveLevel;

	//Will save data layer and activate them on streaming map when game loaded
	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Save")
	bool bSaveDataLayer;


};

