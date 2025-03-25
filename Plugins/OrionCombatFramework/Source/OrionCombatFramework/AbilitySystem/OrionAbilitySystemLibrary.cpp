// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionAbilitySystemLibrary.h"

#include "Kismet/GameplayStatics.h"
#include "OrionCombatFramework/Player/OrionPlayerState.h"
#include "OrionCombatFramework/UI/HUD/OrionHUD.h"
#include "OrionCombatFramework/UI/WidgetController/OrionWidgetController.h"
#include "../Character/OrionCharacterBase.h"
#include "OrionAbilityTypes.h"
#include "ScalableFloat.h"

UOverlayWidgetController* UOrionAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (AOrionHUD* OrionHUD = Cast<AOrionHUD>(PC->GetHUD()))
		{
			AOrionPlayerState* PS = PC->GetPlayerState<AOrionPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
			return OrionHUD->GetOverlayWidgetController(WidgetControllerParams);
		}
	}
	return nullptr;
}

UAttributeMenuWidgetController* UOrionAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (AOrionHUD* OrionHUD = Cast<AOrionHUD>(PC->GetHUD()))
		{
			AOrionPlayerState* PS = PC->GetPlayerState<AOrionPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
			return OrionHUD->GetAttributeMenuWidgetController(WidgetControllerParams);
		}
	}
	return nullptr;
}

bool UOrionAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FOrionGameplayEffectContext* OrionEffectContext = static_cast<const FOrionGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return OrionEffectContext->IsBlockedHit();
	}
	return false;
}

bool UOrionAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FOrionGameplayEffectContext* OrionEffectContext = static_cast<const FOrionGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return OrionEffectContext->IsCriticalHit();
	}
	return false;
}

void UOrionAbilitySystemLibrary::SetIsBlockedHit(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
	if (FOrionGameplayEffectContext* OrionEffectContext = static_cast<FOrionGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		OrionEffectContext->SetIsBlockedHit(bInIsBlockedHit);
	}
}

void UOrionAbilitySystemLibrary::SetIsCriticalHit(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit)
{
	if (FOrionGameplayEffectContext* OrionEffectContext = static_cast<FOrionGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		OrionEffectContext->SetIsCriticalHit(bInIsCriticalHit);
	}
}

int32 UOrionAbilitySystemLibrary::GetXPRewardFromLevel(UObject* WorldContextObject, int32 CharacterLevel)
{
	AOrionCharacterBase* OrionCharacter = Cast<AOrionCharacterBase>(WorldContextObject);
	const float XPReward = OrionCharacter->GetXPReward().GetValueAtLevel(CharacterLevel); 
 	return static_cast<int32>(XPReward);
	//return int32();
}