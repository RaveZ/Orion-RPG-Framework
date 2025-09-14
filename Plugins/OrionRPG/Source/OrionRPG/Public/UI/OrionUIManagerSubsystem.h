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
		//WASD causing the the enhanced input not working the first time playing. need to find a way to register and unregister whenever system try to open menu.
		/*KeyEventRules.Emplace(EKeys::A, EUINavigation::Left);
		KeyEventRules.Emplace(EKeys::D, EUINavigation::Right);
		KeyEventRules.Emplace(EKeys::W, EUINavigation::Up);
		KeyEventRules.Emplace(EKeys::S, EUINavigation::Down);*/
		KeyEventRules.Emplace(EKeys::Left, EUINavigation::Left);
		KeyEventRules.Emplace(EKeys::Right, EUINavigation::Right);
		KeyEventRules.Emplace(EKeys::Up, EUINavigation::Up);
		KeyEventRules.Emplace(EKeys::Down, EUINavigation::Down);
		KeyEventRules.Emplace(EKeys::Gamepad_DPad_Left, EUINavigation::Left);
		KeyEventRules.Emplace(EKeys::Gamepad_DPad_Right, EUINavigation::Right);
		KeyEventRules.Emplace(EKeys::Gamepad_DPad_Up, EUINavigation::Up);
		KeyEventRules.Emplace(EKeys::Gamepad_DPad_Down, EUINavigation::Down);

		KeyEventRules.Emplace(EKeys::Gamepad_LeftShoulder, EUINavigation::Previous);
		KeyEventRules.Emplace(EKeys::Gamepad_RightShoulder, EUINavigation::Next);
		KeyEventRules.Emplace(EKeys::X, EUINavigation::Previous);
		KeyEventRules.Emplace(EKeys::C, EUINavigation::Next);
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
