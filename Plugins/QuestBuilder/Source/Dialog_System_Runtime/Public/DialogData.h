// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Dialog.h"
#include "DialogData.generated.h"



/** Dialog Data To Be Saved */
USTRUCT(BlueprintType)
struct FDialogData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "ID", MakeStructureDefaultValue = "None"), Category = DialogData)
	FName CurrentNodeID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "DialogState"), Category = DialogData)
	EDialogState DialogState = EDialogState::E_Locked;

	/** List of All Visited Node IDs in Sequence*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = DialogData)
	TArray<FName> VisitedNodeIDs;
};

USTRUCT(BlueprintType)
struct FDialogDataHandle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DialogDataHandle, meta = (GetOptions = "Dialog_System_Runtime.DialogBuilderFunctionLibrary.GetAllDialogID"))
	FName ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DialogDataHandle, meta = (GetOptions = "Dialog_System_Runtime.DialogBuilderFunctionLibrary.GetNodeIDFromCachedDialog"))
	FName NodeID;

	/** Returns true if this handle is specifically pointing to nothing */
	bool IsNull() const
	{
		return ID.IsNone() && NodeID.IsNone();
	}

	DIALOG_SYSTEM_RUNTIME_API void PostSerialize(const FArchive& Ar);
};

template<>
struct TStructOpsTypeTraits< FDialogDataHandle > : public TStructOpsTypeTraitsBase2< FDialogDataHandle >
{
	enum
	{
		WithPostSerialize = true,
	};
};

