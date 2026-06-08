// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "OrionAttributeSet.h"
#include "NativeGameplayTags.h"
#include "OrionDefaultAttributeSet.generated.h"

class UObject;

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_Damage);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_DamageImmunity);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_DamageSelfDestruct);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_FellOutOfWorld);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Orion_Damage_Message);
/**
 * 
 */
UCLASS(BlueprintType)
class ORIONCOMBAT_API UOrionDefaultAttributeSet : public UOrionAttributeSet
{
	GENERATED_BODY()
public:
	UOrionDefaultAttributeSet();
	
	ATTRIBUTE_ACCESSORS(UOrionDefaultAttributeSet, Health);
	ATTRIBUTE_ACCESSORS(UOrionDefaultAttributeSet, Mana);
	ATTRIBUTE_ACCESSORS(UOrionDefaultAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UOrionDefaultAttributeSet, MaxMana);
	ATTRIBUTE_ACCESSORS(UOrionDefaultAttributeSet, Healing);
	ATTRIBUTE_ACCESSORS(UOrionDefaultAttributeSet, Damage);
	
	// Delegate when health changes due to damage/healing, some information may be missing on the client
	mutable FOrionAttributeEvent OnHealthChanged;

	// Delegate when max health changes
	mutable FOrionAttributeEvent OnMaxHealthChanged;

	// Delegate to broadcast when the health attribute reaches zero
	mutable FOrionAttributeEvent OnOutOfHealth;
	
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
	
protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldValue);
	
private:
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Orion|DefaultAttribute", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Health;

	// The current max health attribute.  Max health is an attribute since gameplay effects can modify it
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Orion|DefaultAttribute", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxHealth;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Orion|DefaultAttribute", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Mana;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Orion|DefaultAttribute", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxMana;
	
	// Used to track when the health reaches 0.
	bool bOutOfHealth;

	// Store the health before any changes 
	float MaxHealthBeforeAttributeChange;
	float HealthBeforeAttributeChange;
	
	// -------------------------------------------------------------------
	//	Meta Attribute (please keep attributes that aren't 'stateful' below 
	// -------------------------------------------------------------------

	// Incoming healing. This is mapped directly to +Health
	UPROPERTY(BlueprintReadOnly, Category = "Orion|DefaultAttribute", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData Healing;

	// Incoming damage. This is mapped directly to -Health
	UPROPERTY(BlueprintReadOnly, Category = "Orion|DefaultAttribute", Meta=(HideFromModifiers, AllowPrivateAccess=true))
	FGameplayAttributeData Damage;
	

};
