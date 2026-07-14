// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "OrionAttributeSet.h"
#include "NativeGameplayTags.h"
#include "OrionAttributeSetBase.generated.h"

class UObject;

/**
 * 
 */
UCLASS(BlueprintType)
class ORIONCOMBAT_API UOrionAttributeSetBase : public UOrionAttributeSet
{
	GENERATED_BODY()
public:
	UOrionAttributeSetBase();
	
	ATTRIBUTE_ACCESSORS(UOrionAttributeSetBase, Health);
	ATTRIBUTE_ACCESSORS(UOrionAttributeSetBase, Mana);
	ATTRIBUTE_ACCESSORS(UOrionAttributeSetBase, Stamina);
	ATTRIBUTE_ACCESSORS(UOrionAttributeSetBase, MaxHealth);
	ATTRIBUTE_ACCESSORS(UOrionAttributeSetBase, MaxMana);
	ATTRIBUTE_ACCESSORS(UOrionAttributeSetBase, MaxStamina);
	ATTRIBUTE_ACCESSORS(UOrionAttributeSetBase, Defense);
	ATTRIBUTE_ACCESSORS(UOrionAttributeSetBase, Healing);
	ATTRIBUTE_ACCESSORS(UOrionAttributeSetBase, Damage);
	
	UFUNCTION(BlueprintCallable, Category = "Orion|DefaultAttribute")
	bool IsAlive() const { return !bOutOfHealth; }
	
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
	void OnRep_Stamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Defense(const FGameplayAttributeData& OldValue);
	
private:
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Orion|DefaultAttribute", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Health;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Orion|DefaultAttribute", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Mana;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "Orion|DefaultAttribute", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Stamina;
	
	// The current max health attribute.  Max health is an attribute since gameplay effects can modify it
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Orion|DefaultAttribute", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxHealth;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Orion|DefaultAttribute", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxMana;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Orion|DefaultAttribute", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxStamina;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "Orion|DefaultAttribute", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Defense;
	
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
