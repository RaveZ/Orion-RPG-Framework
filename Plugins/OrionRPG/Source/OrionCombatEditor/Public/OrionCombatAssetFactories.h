// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/BlueprintFactory.h"
#include "OrionCombatAssetFactories.generated.h"

namespace OrionCombatEditor
{
	void SetOrionRPGAssetCategory(uint32 InCategory);
	uint32 GetOrionRPGAssetCategory();
}

UCLASS(Abstract)
class UOrionCombatBlueprintFactoryBase : public UBlueprintFactory
{
	GENERATED_BODY()

public:
	UOrionCombatBlueprintFactoryBase();

	virtual uint32 GetMenuCategories() const override;
	virtual const TArray<FText>& GetMenuCategorySubMenus() const override;
};

UCLASS()
class UOrionGameplayAbilityFactory : public UOrionCombatBlueprintFactoryBase
{
	GENERATED_BODY()

public:
	UOrionGameplayAbilityFactory();

	virtual FText GetDisplayName() const override;
	virtual FText GetToolTip() const override;
	virtual FString GetDefaultNewAssetName() const override;
};

UCLASS()
class UOrionGameplayEffectFactory : public UOrionCombatBlueprintFactoryBase
{
	GENERATED_BODY()

public:
	UOrionGameplayEffectFactory();

	virtual FText GetDisplayName() const override;
	virtual FText GetToolTip() const override;
	virtual FString GetDefaultNewAssetName() const override;
};

UCLASS()
class UOrionAttributeSetFactory : public UOrionCombatBlueprintFactoryBase
{
	GENERATED_BODY()

public:
	UOrionAttributeSetFactory();

	virtual FText GetDisplayName() const override;
	virtual FText GetToolTip() const override;
	virtual FString GetDefaultNewAssetName() const override;
};
