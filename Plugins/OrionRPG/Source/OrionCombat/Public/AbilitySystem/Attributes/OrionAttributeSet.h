// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystemComponent.h"
#include "OrionAttributeSet.generated.h"

class UOrionAbilitySystemComponent;
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/** 
 * Delegate used to broadcast attribute events, some of these parameters may be null on clients: 
 * @param EffectInstigator	The original instigating actor for this event
 * @param EffectCauser		The physical actor that caused the change
 * @param EffectSpec		The full effect spec for this change
 * @param EffectMagnitude	The raw magnitude, this is before clamping
 * @param OldValue			The value of the attribute before it was changed
 * @param NewValue			The value after it was changed
*/
DECLARE_MULTICAST_DELEGATE_SixParams(FOrionAttributeEvent, AActor* /*EffectInstigator*/, AActor* /*EffectCauser*/, const FGameplayEffectSpec* /*EffectSpec*/, float /*EffectMagnitude*/, float /*OldValue*/, float /*NewValue*/);



/**
 * FLyraAbilitySet_GrantedHandles
 *
 *	Data used to store handles to what has been granted by the ability set.
 */
USTRUCT(BlueprintType)
struct FOrionAttributeClampData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayAttribute AttributeToClamp;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayAttribute MaxAttributeValue;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MinValue = 0.0f;
	
};


/**
 * Base attribute set class for Orion combat module.
 */
UCLASS(BlueprintType, Blueprintable)
class ORIONCOMBAT_API UOrionAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config")
	TArray<FOrionAttributeClampData> AttributeClampData;
	
public:
	UOrionAttributeSet();
	
	UWorld* GetWorld() const override;
	UOrionAbilitySystemComponent* GetOrionAbilitySystemComponent() const;
	
	virtual void InitFromMetaDataTable(const UDataTable* DataTable) override;
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
	
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnPreGameplayEffectExecute", Category = "OrionAttributeSet")
	void K2_PreGameplayEffectExecute(AActor* Instigator, AActor* Causer, const FGameplayEffectSpec& EffectSpec, float EffectMagnitude);
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnPostGameplayEffectExecute", Category = "OrionAttributeSet")
	void K2_PostGameplayEffectExecute(AActor* Instigator, AActor* Causer, const FGameplayEffectSpec& EffectSpec, float EffectMagnitude);
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnPreAttributeBaseChange", Category = "OrionAttributeSet")
	void K2_PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const;
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnPreAttributeChange", Category = "OrionAttributeSet")
	void K2_PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue);
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnPostAttributeChange", Category = "OrionAttributeSet")
	void K2_PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue);
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnClampAttribute", Category = "OrionAttributeSet")
	void K2_ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
	
private:
	FString GetFormattedClassNameForDataTable() const;
	

};
