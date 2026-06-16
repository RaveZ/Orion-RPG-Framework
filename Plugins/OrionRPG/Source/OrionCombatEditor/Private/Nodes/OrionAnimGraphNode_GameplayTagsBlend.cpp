// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "Nodes/OrionAnimGraphNode_GameplayTagsBlend.h"
#include "GameplayTagsManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OrionAnimGraphNode_GameplayTagsBlend)

#define LOCTEXT_NAMESPACE "OrionAnimGraphNode_GameplayTagsBlend"

UOrionAnimGraphNode_GameplayTagsBlend::UOrionAnimGraphNode_GameplayTagsBlend()
{
	Node.AddPose();
}

void UOrionAnimGraphNode_GameplayTagsBlend::PostEditChangeProperty(FPropertyChangedEvent& ChangedEvent)
{
	if (ChangedEvent.GetPropertyName() == GET_MEMBER_NAME_STRING_VIEW_CHECKED(FOrionAnimNode_GameplayTagsBlend, Tags))
	{
		ReconstructNode();
	}

	Super::PostEditChangeProperty(ChangedEvent);
}

FText UOrionAnimGraphNode_GameplayTagsBlend::GetNodeTitle(const ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Title", "Blend Poses by Gameplay Tag");
}

FText UOrionAnimGraphNode_GameplayTagsBlend::GetTooltipText() const
{
	return LOCTEXT("Tooltip", "Blend Poses by Gameplay Tag");
}

void UOrionAnimGraphNode_GameplayTagsBlend::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& PreviousPins) 
{
	Node.RefreshPosePins();

	Super::ReallocatePinsDuringReconstruction(PreviousPins);
}

FText UOrionAnimGraphNode_GameplayTagsBlend::GetMenuCategory() const
{
	return LOCTEXT("Category", "DO3");
}

FString UOrionAnimGraphNode_GameplayTagsBlend::GetNodeCategory() const
{
	return GetMenuCategory().ToString();
}

void UOrionAnimGraphNode_GameplayTagsBlend::CustomizePinData(UEdGraphPin* Pin, const FName SourcePropertyName, const int32 PinIndex) const
{
	Super::CustomizePinData(Pin, SourcePropertyName, PinIndex);

	bool bBlendPosePin;
	bool bBlendTimePin;
	GetBlendPinProperties(Pin, bBlendPosePin, bBlendTimePin);

	if (!bBlendPosePin && !bBlendTimePin)
	{
		return;
	}
	FName TagName = NAME_None;
	const int32 TagIndex = PinIndex - 1;
	if (Node.Tags.IsValidIndex(TagIndex))
	{
		const auto TagNode{ UGameplayTagsManager::Get().FindTagNode(Node.Tags[TagIndex])};

		TagName = TagNode.IsValid() ? TagNode->GetSimpleTagName() : NAME_None;
	}

	Pin->PinFriendlyName = PinIndex <= 0
		? LOCTEXT("Default", "Default")
		: PinIndex > Node.Tags.Num()
		? LOCTEXT("Invalid", "Invalid")
		: FText::AsCultureInvariant(TagName.ToString());

	if (bBlendPosePin)
	{
		static const FTextFormat BlendPosePinFormat{LOCTEXT("Pose", "{PinName} Pose")};

		Pin->PinFriendlyName = FText::Format(BlendPosePinFormat, {{FString{TEXTVIEW("PinName")}, Pin->PinFriendlyName}});
	}
	else if (bBlendTimePin)
	{
		static const FTextFormat BlendTimePinFormat{LOCTEXT("BlendTime", "{PinName} Blend Time")};

		Pin->PinFriendlyName = FText::Format(BlendTimePinFormat, {{FString{TEXTVIEW("PinName")}, Pin->PinFriendlyName}});
	}
}

void UOrionAnimGraphNode_GameplayTagsBlend::GetBlendPinProperties(const UEdGraphPin* Pin, bool& bBlendPosePin, bool& bBlendTimePin)
{
	const auto PinFullName{Pin->PinName.ToString()};
	const auto SeparatorIndex{PinFullName.Find(TEXTVIEW("_"), ESearchCase::CaseSensitive)};

	if (SeparatorIndex <= 0)
	{
		bBlendPosePin = false;
		bBlendTimePin = false;
		return;
	}

	const auto PinName{PinFullName.Left(SeparatorIndex)};
	bBlendPosePin = PinName == TEXTVIEW("BlendPose");
	bBlendTimePin = PinName == TEXTVIEW("BlendTime");
}

#undef LOCTEXT_NAMESPACE
