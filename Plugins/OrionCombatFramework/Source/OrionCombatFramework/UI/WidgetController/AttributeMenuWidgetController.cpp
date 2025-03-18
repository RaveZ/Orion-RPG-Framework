// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeMenuWidgetController.h"
#include "OrionCombatFramework/AbilitySystem/OrionAttributeSet.h"
#include "OrionCombatFramework/AbilitySystem/Data/AttributeInfo.h"
#include "OrionCombatFramework/OrionGameplayTags.h"

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
}

void UAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const
{
    FOrionAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
    Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
    AttributeInfoDelegate.Broadcast(Info);
}
