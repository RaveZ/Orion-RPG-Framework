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


USTRUCT(BlueprintType)
struct FParticipantInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog", meta = (Categories = "Dialog.Participant"))
	FGameplayTag ParticipantTag;

	/** Your Participant Name*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Dialog")
	FText ParticipantName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog", meta = (FullyExpand = true))
	TSubclassOf<class AActor> ParticipantClassOverride;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog")
	FLinearColor NodeColor;
};


UENUM(BlueprintType)
enum class ESelectionTimeLimit : uint8
{
	E_NoTimeLimit		UMETA(DisplayName = "No Time Limit"),
	E_HasTimeLimit		UMETA(DisplayName = "Has Time Limit")
};
