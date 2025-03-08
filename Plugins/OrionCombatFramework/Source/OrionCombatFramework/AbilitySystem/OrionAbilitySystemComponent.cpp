

#include "OrionAbilitySystemComponent.h"


UOrionAbilitySystemComponent::UOrionAbilitySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	
}

void UOrionAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &ThisClass::EffectApplied);
}


void UOrionAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UOrionAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);
	EffectAssetTags.Broadcast(TagContainer);
}


void UOrionAbilitySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

