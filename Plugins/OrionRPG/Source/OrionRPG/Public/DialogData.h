// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "UObject/NoExportTypes.h"
#include "DialogData.generated.h"

/** Dialog Data To Be Saved */
USTRUCT()
struct FDialogData
{
	GENERATED_BODY()
public:
};

USTRUCT(BlueprintType)
struct FDialogDataHandle
{
	GENERATED_BODY()

public:
	/**Wrapper Data to pick dialog id using picker*/
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "Dialog Data Handle", meta = (FullyExpand = true, AllowPrivateAccess = true))
	TObjectPtr<UDialogHandle> DialogDataHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = DialogDataHandle, meta = (AllowPrivateAccess = true))
	FName DialogID;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = DialogDataHandle, meta = (AllowPrivateAccess = true))
	FName NodeID;

	// Constructor
	FDialogDataHandle();

	ORIONRPG_API void PostSerialize(const FArchive& Ar);
};

template<>
struct TStructOpsTypeTraits< FDialogDataHandle > : public TStructOpsTypeTraitsBase2< FDialogDataHandle >
{
	enum
	{
		WithPostSerialize = true,
	};
};


UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced, AutoExpandCategories = ("Default"))
class ORIONRPG_API UDialogHandle : public UObject
{
	GENERATED_BODY()

	public:
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DialogDataHandle, meta = (GetOptions = "GetAllDialogID"))
		FName DialogID;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DialogDataHandle, meta = (GetOptions = "GetAllNodeID"))
		FName NodeID;


		UFUNCTION(CallInEditor, BlueprintCallable)
		virtual TArray<FName> GetAllDialogID();

		UFUNCTION(CallInEditor)
		virtual TArray<FName> GetAllNodeID();
};