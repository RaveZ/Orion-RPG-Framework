// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogData.h"
#include "UObject/NoExportTypes.h"
#include "DialogDefinition.generated.h"

UCLASS(abstract, Blueprintable, BlueprintType)
class ORIONRPG_API UDialogDefinition : public UObject
{
	GENERATED_BODY()

public:
	UDialogDefinition();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialog")
	FGuid ID;

	void RefreshID()
	{
		if (!HasAnyFlags(RF_ClassDefaultObject) && !ID.IsValid())
		{
			ID = FGuid::NewGuid();
		}
	}
};

UCLASS(Blueprintable, BlueprintType)
class ORIONRPG_API UDialogParticipant : public UDialogDefinition
{
	GENERATED_BODY()

public:
	UDialogParticipant();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
		FParticipantInfo ParticipantInfo;

};

UCLASS(Blueprintable, BlueprintType)
class ORIONRPG_API UDialogProp : public UDialogDefinition
{
	GENERATED_BODY()

public:
	UDialogProp();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	FPropInfo PropInfo;

};

UCLASS(Blueprintable, BlueprintType)
class ORIONRPG_API UDialogCamera : public UDialogDefinition
{
	GENERATED_BODY()

public:
	UDialogCamera();


};


