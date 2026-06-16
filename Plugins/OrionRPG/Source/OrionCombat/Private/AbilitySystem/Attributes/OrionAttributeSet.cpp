// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "AbilitySystem/Attributes//OrionAttributeSet.h"
#include "GameplayEffectTypes.h" 
#include "AbilitySystemLog.h"
#include "GameplayEffectExtension.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "AbilitySystem/OrionAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

UOrionAttributeSet::UOrionAttributeSet()
{
}

UWorld* UOrionAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

UOrionAbilitySystemComponent* UOrionAttributeSet::GetOrionAbilitySystemComponent() const
{
	return Cast<UOrionAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}


bool UOrionAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	AActor* Instigator = Data.EffectSpec.GetEffectContext().GetInstigator();
	AActor* Causer = Data.EffectSpec.GetEffectContext().GetEffectCauser();
	float EffectMagnitude = 0.0f;
    
	K2_PreGameplayEffectExecute(Instigator, Causer, Data.EffectSpec, EffectMagnitude);
	
	return Super::PreGameplayEffectExecute(Data);
}

void UOrionAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	AActor* Instigator = Data.EffectSpec.GetEffectContext().GetInstigator();
	AActor* Causer = Data.EffectSpec.GetEffectContext().GetEffectCauser();
	float EffectMagnitude = 0.0f; 
    
	K2_PostGameplayEffectExecute(Instigator, Causer, Data.EffectSpec, EffectMagnitude);
}

void UOrionAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	
	ClampAttribute(Attribute, NewValue);
	K2_PreAttributeBaseChange(Attribute, NewValue);
}

void UOrionAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	ClampAttribute(Attribute, NewValue);
	K2_PreAttributeChange(Attribute, NewValue);
}

void UOrionAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	K2_PostAttributeChange(Attribute, OldValue, NewValue);
}

void UOrionAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	for (auto& ClampData : AttributeClampData)
	{
		if (Attribute == ClampData.AttributeToClamp)
		{
			NewValue = FMath::Clamp(NewValue, ClampData.MinValue, ClampData.MaxAttributeValue.GetNumericValue(this));
		}
	}
	K2_ClampAttribute(Attribute, NewValue);
	
}

void UOrionAttributeSet::InitFromMetaDataTable(const UDataTable* DataTable)
{
	if (!DataTable)
	{
		UE_LOG(LogAbilitySystem, Warning, TEXT("InitFromMetaDataTable called with null DataTable"));
		return;
	}

	static const FString Context = FString(TEXT("UOrionAttributeSet::InitFromMetaDataTable"));
	
	// Get the formatted class name (DisplayName or stripped name)
	const FString FormattedClassName = GetFormattedClassNameForDataTable();
	const FString RawClassName = GetClass()->GetName();
	const FString StrippedClassName = RawClassName.EndsWith(TEXT("_C")) ? RawClassName.Left(RawClassName.Len() - 2) : RawClassName;

	for (TFieldIterator<FProperty> It(GetClass(), EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		FProperty* Property = *It;
		
		// Helper lambda to try finding attribute in DataTable with different naming strategies
		auto FindAttributeMetaData = [&](const FString& ClassNameToTry) -> FAttributeMetaData*
		{
			FString RowNameStr = FString::Printf(TEXT("%s.%s"), *ClassNameToTry, *Property->GetName());
			return DataTable->FindRow<FAttributeMetaData>(FName(*RowNameStr), Context, false);
		};
		
		FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property);
		if (NumericProperty)
		{
			FAttributeMetaData* MetaData = nullptr;
			FString UsedRowName;
			
			// 1. Try Display Name if available (from FormattedClassName)
			if (!FormattedClassName.Equals(StrippedClassName))
			{
				MetaData = FindAttributeMetaData(FormattedClassName);
				if (MetaData) UsedRowName = FString::Printf(TEXT("%s.%s"), *FormattedClassName, *Property->GetName());
			}
			
			// 2. Try stripped class name (without _C)
			if (!MetaData)
			{
				MetaData = FindAttributeMetaData(StrippedClassName);
				if (MetaData) UsedRowName = FString::Printf(TEXT("%s.%s"), *StrippedClassName, *Property->GetName());
			}
			
			// 3. Try raw class name (with _C) as fallback
			if (!MetaData)
			{
				MetaData = FindAttributeMetaData(RawClassName);
				if (MetaData) UsedRowName = FString::Printf(TEXT("%s.%s"), *RawClassName, *Property->GetName());
			}
			
			// 4. Try property name only (without class prefix) as last resort
			if (!MetaData)
			{
				MetaData = DataTable->FindRow<FAttributeMetaData>(FName(*Property->GetName()), Context, false);
				if (MetaData) UsedRowName = Property->GetName();
			}
			
			if (MetaData)
			{
				void* Data = NumericProperty->ContainerPtrToValuePtr<void>(this);
				NumericProperty->SetFloatingPointPropertyValue(Data, MetaData->BaseValue);
				
				UE_LOG(LogAbilitySystem, Verbose, TEXT("Initialized numeric attribute %s with value %f (using row: %s)"), 
					*Property->GetName(), MetaData->BaseValue, *UsedRowName);
			}
			else
			{
				UE_LOG(LogAbilitySystem, Warning, TEXT("Could not find DataTable row for attribute: %s (tried: DisplayName='%s', Stripped='%s', Raw='%s', PropertyOnly='%s')"), 
					*Property->GetName(), *FormattedClassName, *StrippedClassName, *RawClassName, *Property->GetName());
			}
		}
		else if (FGameplayAttribute::IsGameplayAttributeDataProperty(Property))
		{
			FAttributeMetaData* MetaData = nullptr;
			FString UsedRowName;
			
			// Try different naming strategies in order of preference
			// 1. Try Display Name if available
			if (!FormattedClassName.Equals(StrippedClassName))
			{
				MetaData = FindAttributeMetaData(FormattedClassName);
				if (MetaData) UsedRowName = FString::Printf(TEXT("%s.%s"), *FormattedClassName, *Property->GetName());
			}
			
			// 2. Try stripped class name (without _C)
			if (!MetaData)
			{
				MetaData = FindAttributeMetaData(StrippedClassName);
				if (MetaData) UsedRowName = FString::Printf(TEXT("%s.%s"), *StrippedClassName, *Property->GetName());
			}
			
			// 3. Try raw class name (with _C) as fallback
			if (!MetaData)
			{
				MetaData = FindAttributeMetaData(RawClassName);
				if (MetaData) UsedRowName = FString::Printf(TEXT("%s.%s"), *RawClassName, *Property->GetName());
			}
			
			// 4. Try property name only (without class prefix) as last resort
			if (!MetaData)
			{
				MetaData = DataTable->FindRow<FAttributeMetaData>(FName(*Property->GetName()), Context, false);
				if (MetaData) UsedRowName = Property->GetName();
			}
			
			if (MetaData)
			{
				FStructProperty* StructProperty = CastField<FStructProperty>(Property);
				check(StructProperty);
				FGameplayAttributeData* DataPtr = StructProperty->ContainerPtrToValuePtr<FGameplayAttributeData>(this);
				check(DataPtr);
				DataPtr->SetBaseValue(MetaData->BaseValue);
				DataPtr->SetCurrentValue(MetaData->BaseValue);
				
				UE_LOG(LogAbilitySystem, Verbose, TEXT("Initialized gameplay attribute data %s with value %f (using row: %s)"), 
					*Property->GetName(), MetaData->BaseValue, *UsedRowName);
			}
			else
			{
				UE_LOG(LogAbilitySystem, Warning, TEXT("Could not find DataTable row for gameplay attribute: %s (tried: DisplayName='%s', Stripped='%s', Raw='%s', PropertyOnly='%s')"), 
					*Property->GetName(), *FormattedClassName, *StrippedClassName, *RawClassName, *Property->GetName());
			}
		}
	}
}


FString UOrionAttributeSet::GetFormattedClassNameForDataTable() const
{
	FString ClassName = GetClass()->GetName();
	
	// Remove _C suffix for Blueprint classes
	if (ClassName.EndsWith(TEXT("_C")))
	{
		ClassName = ClassName.Left(ClassName.Len() - 2);
	}
	
	// Check if this is a Blueprint and get its display name if available
	if (UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		if (UBlueprint* Blueprint = Cast<UBlueprint>(BPClass->ClassGeneratedBy))
		{
			// Try to get Display Name from Blueprint's generated class
			if (Blueprint->GeneratedClass)
			{
				const FString DisplayName = Blueprint->GeneratedClass->GetMetaData(TEXT("DisplayName"));
				if (!DisplayName.IsEmpty())
				{
					return DisplayName; // Use Display Name if available
				}
			}
			
			// Alternative: Check Blueprint asset's metadata
			const FString BlueprintDisplayName = Blueprint->GeneratedClass->GetMetaData(TEXT("DisplayName"));
			if (!BlueprintDisplayName.IsEmpty())
			{
				return BlueprintDisplayName;
			}
		}
	}
	
	return ClassName;
}


