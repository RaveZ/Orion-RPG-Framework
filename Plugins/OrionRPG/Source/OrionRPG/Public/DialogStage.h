// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogData.h"
#include "NativeGameplayTags.h"
#include "UObject/NoExportTypes.h"
#include "DialogStage.generated.h"

class UDialogBuilderGraph;

UCLASS(Blueprintable, BlueprintType, EditInlineNew, AutoExpandCategories = "Detail")
class ORIONRPG_API UDialogSequenceSlot : public UObject
{
	GENERATED_BODY()

public:
	UDialogSequenceSlot();

	UPROPERTY(BlueprintReadOnly, Category = "Detail")
	TObjectPtr<UDialogBuilderGraph> OwningDialogGraph;

	//id used for binding reference in sequence
	UPROPERTY(BlueprintReadOnly, Category = "Detail", meta = (DisplayPriority = -1))
	FGuid ID;

	UPROPERTY(BlueprintReadOnly, Category = "Detail")
	TObjectPtr<class UDialogDefinition> DialogDefinition;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail")
	FVector SlotLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail")
	FRotator SlotRotation;

	FName GetID() const
	{
		FName OutName = FName(*ID.ToString());
		return OutName;
	}

	FGameplayTag GetActorTag() const;

	void SetDialogDefinition(UDialogDefinition* InDialogDefinition);

#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface
#endif
};



UCLASS(Blueprintable, BlueprintType, EditInlineNew, AutoExpandCategories = "Detail")
class ORIONRPG_API UDialogStage : public UObject
{
	GENERATED_BODY()

public:
	UDialogStage();

	//Parent template for this stage
	UPROPERTY(BlueprintReadOnly, Category = "Detail")
	TObjectPtr<UDialogStage> StageTemplate;

	UPROPERTY(BlueprintReadOnly, Category = "Detail")
	TObjectPtr<UDialogBuilderGraph> OwningDialogGraph;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Detail")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail")
	FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail")
	FRotator Rotation;

	UPROPERTY(Instanced, BlueprintReadOnly, Category = "Detail")
	TArray<TObjectPtr<class UDialogSequenceSlot>> Slots;

	UPROPERTY(Instanced, BlueprintReadOnly, Category = "Detail")
	TArray<TObjectPtr<class UDialogSequenceSlot>> CameraSlots;


#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface
#endif
};

