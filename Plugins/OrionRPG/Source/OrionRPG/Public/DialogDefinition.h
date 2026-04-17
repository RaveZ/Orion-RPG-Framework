// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogData.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/Pawn.h"
#include "DialogDefinition.generated.h"


UCLASS(abstract, Blueprintable, BlueprintType)
class ORIONRPG_API UDialogDefinition : public UObject
{
	GENERATED_BODY()

public:
	UDialogDefinition();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialog Definition", meta = (DisplayPriority = -1))
	FGuid ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog Definition", meta=(Categories = "Dialog"))
	FGameplayTag Tag;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Dialog Definition")
	FText DisplayName;

public:

	virtual FText GetDisplayName() const { return DisplayName; }
	virtual TSubclassOf< class AActor> GetActorClass() const { return nullptr; }
	FGuid GetOrCreateID();

};

/**Pivot for all dialog definition*/
UCLASS(Blueprintable, BlueprintType)
class ORIONRPG_API UDialogPivot : public UDialogDefinition
{
	GENERATED_BODY()
public:
	UDialogPivot();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog Definition", meta = (DisplayAfter = "DisplayName"))
	TSubclassOf<class AActor> ActorClass;

	virtual TSubclassOf< class AActor> GetActorClass() const { return ActorClass; }
};


UCLASS(Blueprintable, BlueprintType)
class ORIONRPG_API UDialogParticipant : public UDialogDefinition
{
	GENERATED_BODY()

public:
	UDialogParticipant();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog Definition", meta = (DisplayAfter = "DisplayName"))
	TSubclassOf<class APawn> PawnClass;

	virtual TSubclassOf< class AActor> GetActorClass() const { return PawnClass.Get(); }
};

UCLASS(Blueprintable, BlueprintType)
class ORIONRPG_API UDialogPlayerParticipant : public UDialogParticipant
{
	GENERATED_BODY()

public:
	UDialogPlayerParticipant();


	virtual FText GetDisplayName() const { return FText::FromString("PLAYER - " + DisplayName.ToString()); }
};


UCLASS(Blueprintable, BlueprintType)
class ORIONRPG_API UDialogProp : public UDialogDefinition
{
	GENERATED_BODY()

public:
	UDialogProp();


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog Definition", meta = (DisplayAfter = "DisplayName"))
	TSubclassOf<class AActor> PropClass;

	virtual TSubclassOf< class AActor> GetActorClass() const { return PropClass; }
};

UCLASS(Blueprintable, BlueprintType)
class ORIONRPG_API UDialogLight : public UDialogDefinition
{
	GENERATED_BODY()

public:
	UDialogLight();


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog Definition", meta = (DisplayAfter = "DisplayName"))
	TSubclassOf<class AActor> LightClass;
	virtual TSubclassOf< class AActor> GetActorClass() const { return LightClass; }
};

UCLASS(Blueprintable, BlueprintType)
class ORIONRPG_API UDialogCamera : public UDialogDefinition
{
	GENERATED_BODY()

public:
	UDialogCamera();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialog Definition", meta = (DisplayAfter = "DisplayName"))
	TSubclassOf<class AActor> CameraClass;

	virtual TSubclassOf< class AActor> GetActorClass() const { return CameraClass; }

};


