// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "AttributeMenuWidgetController.h"
#include "OrionCombatFramework/AbilitySystem/OrionAttributeSet.h"
#include "OrionCombatFramework/AbilitySystem/Data/AttributeInfo.h"
#include "OrionCombatFramework/OrionGameplayTags.h"
#include "OrionCombatFramework/AbilitySystem/OrionAbilitySystemComponent.h"
#include "../../Player/OrionPlayerState.h"

void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
    UOrionAttributeSet* AS = CastChecked<UOrionAttributeSet>(AttributeSet);
    check(AttributeInfo);

    for (auto& Pair : AS->TagsToAttributes)
    {
        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
            [this, Pair](const FOnAttributeChangeData& Data)
            {
                BroadcastAttributeInfo(Pair.Key, Pair.Value());
            }
        );
    }
    AOrionPlayerState* OrionPlayerState = CastChecked<AOrionPlayerState>(PlayerState);
    OrionPlayerState->OnAttributePointsChangedDelegate.AddLambda(
        [this](int32 Points)
        {
            AttributePointsChangedDelegate.Broadcast(Points);
        }
    );
}

void UAttributeMenuWidgetController::BroadcastInitialValues()
{
    UOrionAttributeSet* AS = CastChecked<UOrionAttributeSet>(AttributeSet);
    check(AttributeInfo);

    for (auto& Pair : AS->TagsToAttributes)
    {
        FOrionAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(Pair.Key);
        Info.AttributeValue = Pair.Value().GetNumericValue(AS);
        BroadcastAttributeInfo(Pair.Key, Pair.Value());
    }

	AOrionPlayerState* OrionPlayerState = CastChecked<AOrionPlayerState>(PlayerState);
	AttributePointsChangedDelegate.Broadcast(OrionPlayerState->GetAttributePoints());
}

void UAttributeMenuWidgetController::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	UOrionAbilitySystemComponent* OrionASC = CastChecked<UOrionAbilitySystemComponent>(AbilitySystemComponent);
	OrionASC->UpgradeAttribute(AttributeTag);
}

void UAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const
{
    FOrionAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
    Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
    AttributeInfoDelegate.Broadcast(Info);
}
