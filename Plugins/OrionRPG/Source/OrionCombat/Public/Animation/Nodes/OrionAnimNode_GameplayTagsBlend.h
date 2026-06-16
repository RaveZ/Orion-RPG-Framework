// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "AnimNodes/AnimNode_BlendListBase.h"
#include "OrionAnimNode_GameplayTagsBlend.generated.h"


USTRUCT()
struct ORIONCOMBAT_API FOrionAnimNode_GameplayTagsBlend : public FAnimNode_BlendListBase
{
	GENERATED_BODY()


protected:
#if WITH_EDITORONLY_DATA
	friend class UOrionAnimGraphNode_GameplayTagsBlend;

	UPROPERTY(EditAnywhere, Category = "Settings", Meta = (FoldProperty, PinShownByDefault))
	FGameplayTag ActiveTag;

	UPROPERTY(EditAnywhere, Category = "Settings", Meta = (FoldProperty, Categories = "Character.Movement"))
	TArray<FGameplayTag> Tags;
#endif

protected:
	virtual int32 GetActiveChildIndex() override;

public:
	const FGameplayTag& GetActiveTag() const;

	const TArray<FGameplayTag>& GetTags() const;

#if WITH_EDITOR
	void RefreshPosePins();
#endif
};
