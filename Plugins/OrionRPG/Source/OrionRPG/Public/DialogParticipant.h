// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogData.h"
#include "UObject/NoExportTypes.h"
#include "DialogParticipant.generated.h"


UCLASS(Blueprintable, BlueprintType)
class ORIONRPG_API UDialogParticipant : public UObject
{
	GENERATED_BODY()

public:
	UDialogParticipant();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
		FParticipantInfo ParticipantInfo;

};
