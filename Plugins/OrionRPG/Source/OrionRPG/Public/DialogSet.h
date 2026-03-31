// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogData.h"
#include "UObject/NoExportTypes.h"
#include "DialogSet.generated.h"

class UDialogBuilderGraph;

UCLASS(Blueprintable, BlueprintType, EditInlineNew, AutoExpandCategories = "Detail")
class ORIONRPG_API UDialogSequenceSlot : public UObject
{
	GENERATED_BODY()

public:
	UDialogSequenceSlot();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detail")
	TObjectPtr<UDialogBuilderGraph> OwningDialogGraph;

	//id used for binding reference in sequence
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detail", meta = (DisplayPriority = -1))
	FGuid ID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detail")
	TObjectPtr<class UDialogDefinition> DialogDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail", meta = (Categories = "Dialog"))
	FGameplayTag ActorTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail")
	FTransform SlotPlacement;

	FName GetID()
	{
		FName OutName = ActorTag.IsValid() ? ActorTag.GetTagName() : FName(*ID.ToString());
		return OutName;
	}
#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface
#endif
};



UCLASS(Blueprintable, BlueprintType, EditInlineNew, AutoExpandCategories = "Detail")
class ORIONRPG_API UDialogSet : public UObject
{
	GENERATED_BODY()

public:
	UDialogSet();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detail")
	TObjectPtr<UDialogBuilderGraph> OwningDialogGraph;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Detail")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail")
	FTransform PlacementTransform;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = "Detail")
	TArray<TObjectPtr<class UDialogSequenceSlot>> Slots;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = "Detail")
	TArray<TObjectPtr<class UDialogSequenceSlot>> CameraSlots;


#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface
#endif
};

