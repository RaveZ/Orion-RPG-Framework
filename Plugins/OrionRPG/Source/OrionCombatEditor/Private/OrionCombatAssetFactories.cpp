// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "OrionCombatAssetFactories.h"

#include "AbilitySystem/Abilities/OrionGameplayAbility.h"
#include "AbilitySystem/Attributes/OrionAttributeSet.h"
#include "AssetTypeCategories.h"
#include "GameplayEffect.h"

#define LOCTEXT_NAMESPACE "OrionCombatAssetFactories"

namespace OrionCombatEditor
{
	static uint32 GOrionRPGAssetCategory = EAssetTypeCategories::Gameplay;

	void SetOrionRPGAssetCategory(uint32 InCategory)
	{
		GOrionRPGAssetCategory = InCategory;
	}

	uint32 GetOrionRPGAssetCategory()
	{
		return GOrionRPGAssetCategory;
	}
}

UOrionCombatBlueprintFactoryBase::UOrionCombatBlueprintFactoryBase()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UBlueprint::StaticClass();
	bSkipClassPicker = true;
	BlueprintType = BPTYPE_Normal;
}

uint32 UOrionCombatBlueprintFactoryBase::GetMenuCategories() const
{
	return OrionCombatEditor::GetOrionRPGAssetCategory();
}

const TArray<FText>& UOrionCombatBlueprintFactoryBase::GetMenuCategorySubMenus() const
{
	static const TArray<FText> SubMenus = { LOCTEXT("AbilitySubMenu", "Ability") };
	return SubMenus;
}

UOrionGameplayAbilityFactory::UOrionGameplayAbilityFactory()
{
	ParentClass = UOrionGameplayAbility::StaticClass();
}

FText UOrionGameplayAbilityFactory::GetDisplayName() const
{
	return LOCTEXT("OrionGameplayAbilityDisplayName", "Orion Gameplay Ability");
}

FText UOrionGameplayAbilityFactory::GetToolTip() const
{
	return LOCTEXT("OrionGameplayAbilityToolTip", "Create a Blueprint subclass of UOrionGameplayAbility.");
}

FString UOrionGameplayAbilityFactory::GetDefaultNewAssetName() const
{
	return TEXT("OrionGameplayAbility");
}

UOrionGameplayEffectFactory::UOrionGameplayEffectFactory()
{
	ParentClass = UGameplayEffect::StaticClass();
}

FText UOrionGameplayEffectFactory::GetDisplayName() const
{
	return LOCTEXT("OrionGameplayEffectDisplayName", "Gameplay Effect");
}

FText UOrionGameplayEffectFactory::GetToolTip() const
{
	return LOCTEXT("OrionGameplayEffectToolTip", "Create a Blueprint subclass of UGameplayEffect.");
}

FString UOrionGameplayEffectFactory::GetDefaultNewAssetName() const
{
	return TEXT("GameplayEffect");
}

UOrionAttributeSetFactory::UOrionAttributeSetFactory()
{
	ParentClass = UOrionAttributeSet::StaticClass();
}

FText UOrionAttributeSetFactory::GetDisplayName() const
{
	return LOCTEXT("OrionAttributeSetDisplayName", "Orion Attribute Set");
}

FText UOrionAttributeSetFactory::GetToolTip() const
{
	return LOCTEXT("OrionAttributeSetToolTip", "Create a Blueprint subclass of UOrionAttributeSet.");
}

FString UOrionAttributeSetFactory::GetDefaultNewAssetName() const
{
	return TEXT("BP_OrionAttributeSet");
}

#undef LOCTEXT_NAMESPACE
