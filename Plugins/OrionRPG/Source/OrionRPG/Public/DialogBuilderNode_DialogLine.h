// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogBuilderNode.h"
#include "DialogBuilderNode_DialogLine.generated.h"

class ORIONRPG_API IDialogNodeSharedDataHelper
{
public:
	void UpdateSharedData(UDialogBuilderNode_DialogLine* Node, TSharedPtr<INameValidatorInterface> NameValidator);
	void MakeSureGuidExists(UDialogBuilderNode_DialogLine* Node);

protected:
	virtual bool CheckIfNodesShouldShareData(const UDialogBuilderNode_DialogLine* NodeA, const UDialogBuilderNode_DialogLine* NodeB) = 0;
	virtual bool CheckIfHasDataToShare(const UDialogBuilderNode_DialogLine* Node) = 0;
	virtual void ShareData(UDialogBuilderNode_DialogLine* NodeWhoWantsToShare, const UDialogBuilderNode_DialogLine* ShareFrom) = 0;
	virtual FString& AccessShareDataName(UDialogBuilderNode_DialogLine* Node) = 0;
	virtual FGuid& AccessShareDataGuid(UDialogBuilderNode_DialogLine* Node) = 0;
};

//////////////////////////////////////////////////////////////////////////
// FDialogSharedParticipantNodeHelper

class ORIONRPG_API FDialogSharedParticipantNodeHelper : public IDialogNodeSharedDataHelper
{
protected:
	virtual bool CheckIfNodesShouldShareData(const UDialogBuilderNode_DialogLine* NodeA, const UDialogBuilderNode_DialogLine* NodeB) override;
	virtual bool CheckIfHasDataToShare(const UDialogBuilderNode_DialogLine* Node) override;
	virtual void ShareData(UDialogBuilderNode_DialogLine* NodeWhoWantsToShare, const UDialogBuilderNode_DialogLine* ShareFrom) override;
	virtual FString& AccessShareDataName(UDialogBuilderNode_DialogLine* Node) override;
	virtual FGuid& AccessShareDataGuid(UDialogBuilderNode_DialogLine* Node) override;
};
//Notify Node Changed to match node title in editor
DECLARE_MULTICAST_DELEGATE(FOnParticipantDataChanged);

UCLASS(Blueprintable)
class ORIONRPG_API UDialogBuilderNode_DialogLine : public UDialogBuilderNode
{
	GENERATED_BODY()

public:

	/**Participant Tag*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog", meta = (Categories = "Dialog.Participant"))
		FGameplayTag ParticipantTag;

	/** Your Participant Name*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Dialog")
		FText ParticipantName;

	UPROPERTY()
		FString SharedParticipantName;

	/** The participant data of this node may be shared */
	UPROPERTY()
		bool bSharedParticipant;

	UPROPERTY()
		FGuid SharedParticipantGuid;

	UPROPERTY()
		int32 SharedParticipantIdx;

	
	FOnParticipantDataChanged OnParticipantDataChanged;
public:

	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostLoad() override;
	virtual void Serialize(FArchive& Ar) override;
	//~ End UObject Interface


public:
#if WITH_EDITOR

	//Participant Sharing
	void MakeParticipantShareable(FString ShareName);
	void UnshareParticipant();
	void UseSharedParticipant(const UDialogBuilderNode_DialogLine* Node);

	void CopyParticipantSettings(const UDialogBuilderNode_DialogLine* SrcNode);
	void PropagateParticipantSettings();
	//Node Setting
	virtual FText GetNodeTitle() const override;
	virtual void SetNodeTitle(const FText& NewTitle);
	virtual FText GetNodeDescription() const override;
#endif
};
