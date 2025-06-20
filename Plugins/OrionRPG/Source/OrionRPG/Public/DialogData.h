// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "UObject/NoExportTypes.h"
#include "NativeGameplayTags.h"
#include "DialogData.generated.h"

class UDialogBuilderNode_DialogLine;

//Gameplay Tags
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Dialog);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Dialog_Participant);


USTRUCT(BlueprintType)
struct FDialogSequenceData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MultiLine = true, NoResetToDefault))
	FText Line;


	FDialogSequenceData()
		: Line(FText::FromString("None"))
	{
	}
};