// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "AnimGraphNode_BlendListBase.h"
#include "Animation/Nodes/OrionAnimNode_GameplayTagsBlend.h"
#include "OrionAnimGraphNode_GameplayTagsBlend.generated.h"


UCLASS()
class ORIONCOMBATEDITOR_API UOrionAnimGraphNode_GameplayTagsBlend : public UAnimGraphNode_BlendListBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FOrionAnimNode_GameplayTagsBlend Node;

public:
	UOrionAnimGraphNode_GameplayTagsBlend();

	virtual void PostEditChangeProperty(FPropertyChangedEvent& ChangedEvent) override;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual FText GetTooltipText() const override;

	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& PreviousPins) override;

	virtual FText GetMenuCategory() const override;

	virtual FString GetNodeCategory() const override;

	virtual void CustomizePinData(UEdGraphPin* Pin, FName SourcePropertyName, int32 PinIndex) const override;

protected:
	static void GetBlendPinProperties(const UEdGraphPin* Pin, bool& bBlendPosePin, bool& bBlendTimePin);
};
