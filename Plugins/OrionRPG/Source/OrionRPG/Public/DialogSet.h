// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogData.h"
#include "UObject/NoExportTypes.h"
#include "DialogSet.generated.h"

UCLASS(Blueprintable, BlueprintType, EditInlineNew, AutoExpandCategories = "Detail")
class ORIONRPG_API UDialogSequenceSlot : public UObject
{
	GENERATED_BODY()

public:
	UDialogSequenceSlot();

	//id used for binding reference in sequence
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detail", meta = (DisplayPriority = -1))
	FGuid ID;

	UPROPERTY(BlueprintReadOnly, Category = "Detail")
	TObjectPtr<class UDialogDefinition> DialogDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail")
	FGameplayTag ActorTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail")
	FTransform SlotPlacement;

};



UCLASS(Blueprintable, BlueprintType, EditInlineNew, AutoExpandCategories = "Detail")
class ORIONRPG_API UDialogSet : public UObject
{
	GENERATED_BODY()

public:
	UDialogSet();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Detail")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail")
	FTransform PlacementTransform;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = "Detail")
	TArray<TObjectPtr<class UDialogSequenceSlot>> Slots;

};

