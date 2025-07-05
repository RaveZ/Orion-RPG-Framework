// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Framework/Application/NavigationConfig.h"
#include "OrionUIManagerSubsystem.generated.h"


class ORIONRPG_API FOrionNavigationConfig : public FNavigationConfig
{
public:
	FOrionNavigationConfig() {
		KeyEventRules.Reset();

		KeyEventRules.Emplace(EKeys::A, EUINavigation::Left);
		KeyEventRules.Emplace(EKeys::D, EUINavigation::Right);
		KeyEventRules.Emplace(EKeys::W, EUINavigation::Up);
		KeyEventRules.Emplace(EKeys::S, EUINavigation::Down);
		KeyEventRules.Emplace(EKeys::Left, EUINavigation::Left);
		KeyEventRules.Emplace(EKeys::Right, EUINavigation::Right);
		KeyEventRules.Emplace(EKeys::Up, EUINavigation::Up);
		KeyEventRules.Emplace(EKeys::Down, EUINavigation::Down);
		KeyEventRules.Emplace(EKeys::Gamepad_DPad_Left, EUINavigation::Left);
		KeyEventRules.Emplace(EKeys::Gamepad_DPad_Right, EUINavigation::Right);
		KeyEventRules.Emplace(EKeys::Gamepad_DPad_Up, EUINavigation::Up);
		KeyEventRules.Emplace(EKeys::Gamepad_DPad_Down, EUINavigation::Down);
	}
};
/**
 *
 */
UCLASS()
class ORIONRPG_API UOrionUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UOrionUIManagerSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	TSharedRef<FOrionNavigationConfig> OrionNavigationConfig;
};
