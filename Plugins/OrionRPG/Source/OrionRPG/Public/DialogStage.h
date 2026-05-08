// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogData.h"
#include "NativeGameplayTags.h"
#include "Engine/PointLight.h"
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

	UDialogStage* GetOwningDialogStage() const;

	FGameplayTag GetActorTag() const;

	void SetDialogDefinition(UDialogDefinition* InDialogDefinition);

	// The serialized template object that stores saved properties
	UPROPERTY(BlueprintReadOnly, Instanced, Category = "Detail")
	TObjectPtr<AActor> ActorTemplate;

	// Helper to create or update the template
	void UpdateActorTemplate(AActor* InActor);

#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface
#endif
};

UCLASS(Blueprintable, BlueprintType, EditInlineNew, AutoExpandCategories = "Detail")
class ORIONRPG_API UDialogSequenceSlot_Light : public UDialogSequenceSlot
{
	GENERATED_BODY()

public:
	UDialogSequenceSlot_Light();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail")
	TSubclassOf<class ALight> LightClass;

};


UCLASS(Blueprintable, BlueprintType, EditInlineNew, AutoExpandCategories = "Detail")
class ORIONRPG_API UDialogStage : public UObject
{
	GENERATED_BODY()

public:
	UDialogStage();


	UPROPERTY(BlueprintReadOnly, Category = "Detail")
	TObjectPtr<UDialogBuilderGraph> OwningDialogGraph;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Detail")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail")
	FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detail")
	FRotator Rotation;

	/**Try to find a safe ground placement when dialog begins*/
	UPROPERTY(BlueprintReadOnly, Category = "Detail")
	bool bSnapToGround;
	
	UPROPERTY()
	bool bIsTemplate = false;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = "Detail", meta = (EditCondition = "bIsTemplate == true", HideEditConditionToggle, EditConditionHides))
	TArray<TObjectPtr<class UDialogSequenceSlot>> Slots;

	UPROPERTY(Instanced, BlueprintReadOnly, Category = "Detail")
	TArray<TObjectPtr<class UDialogSequenceSlot>> CameraSlots;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = "Detail", meta = (EditCondition = "bIsTemplate == true", HideEditConditionToggle, EditConditionHides))
	TArray<TObjectPtr<class UDialogSequenceSlot_Light>> LightSlots;

public:
	class UDialogBuilderNode_DialogSequence* GetOwningDialogSequenceNode() const;
	UDialogStage* GetOwningDialogStage() const;
	void MakeUniqueDialogStageName();


#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface
#endif
};

