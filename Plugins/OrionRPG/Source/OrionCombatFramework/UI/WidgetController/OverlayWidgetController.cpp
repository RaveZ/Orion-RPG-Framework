// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OverlayWidgetController.h"
#include "../../OrionLogChannels.h"
#include "../../Player/OrionPlayerState.h"
#include "OrionCombatFramework/AbilitySystem/OrionAbilitySystemComponent.h"
#include "OrionCombatFramework/AbilitySystem/OrionAttributeSet.h"
#include "OrionCombatFramework/AbilitySystem/Data/AbilityInfo.h"
#include "../../Progression/LevelUpInfo.h"


void UOverlayWidgetController::BroadcastInitialValues()
{
	const UOrionAttributeSet* OrionAttributeSet = CastChecked<UOrionAttributeSet>(AttributeSet);

	OnHealthChanged.Broadcast(OrionAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(OrionAttributeSet->GetMaxHealth());
	OnManaChanged.Broadcast(OrionAttributeSet->GetMana());
	OnMaxManaChanged.Broadcast(OrionAttributeSet->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	AOrionPlayerState* OrionPlayerState = CastChecked<AOrionPlayerState>(PlayerState);
	OrionPlayerState->OnXPChangedDelegate.AddUObject(this, &UOverlayWidgetController::OnXPChanged);
	OrionPlayerState->OnLevelChangedDelegate.AddLambda(
		[this](int32 NewLevel)
		{
			OnPlayerLevelChangedDelegate.Broadcast(NewLevel);
		}
	);

	const UOrionAttributeSet* OrionAttributeSet = CastChecked<UOrionAttributeSet>(AttributeSet);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(OrionAttributeSet->GetHealthAttribute()).AddLambda(
	[this](const FOnAttributeChangeData& Data)
		{
			OnHealthChanged.Broadcast(Data.NewValue);
		}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(OrionAttributeSet->GetMaxHealthAttribute()).AddLambda(
	[this](const FOnAttributeChangeData& Data)
		{
			OnMaxHealthChanged.Broadcast(Data.NewValue);
		}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(OrionAttributeSet->GetManaAttribute()).AddLambda(
	[this](const FOnAttributeChangeData& Data)
		{
			OnManaChanged.Broadcast(Data.NewValue);
		}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(OrionAttributeSet->GetMaxManaAttribute()).AddLambda(
	[this](const FOnAttributeChangeData& Data)
		{
			OnMaxManaChanged.Broadcast(Data.NewValue);
		}
	);

	if (UOrionAbilitySystemComponent* OrionASC = Cast<UOrionAbilitySystemComponent>(AbilitySystemComponent))
	{
		if (OrionASC->bStartupAbilitiesGiven)
		{
			OnInitializeStartupAbilities(OrionASC);
		}
		else
		{
			OrionASC->AbilitiesGivenDelegate.AddUObject(this, &UOverlayWidgetController::OnInitializeStartupAbilities);
		}

		OrionASC->EffectAssetTags.AddLambda(
			[this](const FGameplayTagContainer& AssetTags)
			{
				for (const FGameplayTag& Tag : AssetTags)
				{
					// For example, say that Tag = Message.HealthPotion
					// "Message.HealthPotion".MatchesTag("Message") will return True, "Message".MatchesTag("Message.HealthPotion") will return False
					FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
					if (Tag.MatchesTag(MessageTag))
					{
						const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
						MessageWidgetRowDelegate.Broadcast(*Row);
					}
				}
			}
		);
	}

	
}

void UOverlayWidgetController::OnInitializeStartupAbilities(UOrionAbilitySystemComponent* OrionAbilitySystemComponent)
{
	//TODO Get information about all given abilities, look up their Ability Info, and broadcast it to widgets.
	if (!OrionAbilitySystemComponent->bStartupAbilitiesGiven) return;

	FForEachAbility BroadcastDelegate;
	BroadcastDelegate.BindLambda(
		[this, OrionAbilitySystemComponent](const FGameplayAbilitySpec& AbilitySpec)
		{
			FOrionAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(UOrionAbilitySystemComponent::GetAbilityTagFromSpec(AbilitySpec));
			Info.InputTag = UOrionAbilitySystemComponent::GetInputTagFromSpec(AbilitySpec);
			AbilityInfoDelegate.Broadcast(Info);
		}
	);
	OrionAbilitySystemComponent->ForEachAbility(BroadcastDelegate);
}

void UOverlayWidgetController::OnXPChanged(int32 NewXP) const
{
	const AOrionPlayerState* OrionPlayerState = CastChecked<AOrionPlayerState>(PlayerState);
	const ULevelUpInfo* LevelUpInfo = OrionPlayerState->LevelUpInfo;
	checkf(LevelUpInfo, TEXT("Unabled to find LevelUpInfo. Please fill out OrionPlayerState Blueprint"));

	const int32 Level = LevelUpInfo->FindLevelForXP(NewXP);
	const int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num();

	if (Level <= MaxLevel && Level > 0)
	{
		const int32 LevelUpRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement;
		const int32 PreviousLevelUpRequirement = LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirement;

		const int32 DeltaLevelRequirement = LevelUpRequirement - PreviousLevelUpRequirement;
		const int32 XPForThisLevel = NewXP - PreviousLevelUpRequirement;

		const float XPBarPercent = static_cast<float>(XPForThisLevel) / static_cast<float>(DeltaLevelRequirement);

		OnXPPercentChangedDelegate.Broadcast(XPBarPercent);
	}
}
