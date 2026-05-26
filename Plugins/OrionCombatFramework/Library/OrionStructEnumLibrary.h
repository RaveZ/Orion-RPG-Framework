// Copyright 2025 Ivan Chandra. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "OrionCharacterEnumLibrary.h"

#include "OrionStructEnumLibrary.generated.h"


USTRUCT(BlueprintType)
struct FOrionMovementState
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Movement System")
	EOrionMovementState State = EOrionMovementState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Movement System")
	bool None_ = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Movement System")
	bool Grounded_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Movement System")
	bool InAir_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Movement System")
	bool Mantling_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Movement System")
	bool Ragdoll_ = false;

public:
	FOrionMovementState()
	{
	}

	FOrionMovementState(const EOrionMovementState InitialState) { *this = InitialState; }

	const bool& None() const { return None_; }
	const bool& Grounded() const { return Grounded_; }
	const bool& InAir() const { return InAir_; }
	const bool& Mantling() const { return Mantling_; }
	const bool& Ragdoll() const { return Ragdoll_; }

	operator EOrionMovementState() const { return State; }

	void operator=(const EOrionMovementState NewState)
	{
		State = NewState;
		None_ = State == EOrionMovementState::None;
		Grounded_ = State == EOrionMovementState::Grounded;
		InAir_ = State == EOrionMovementState::InAir;
		Mantling_ = State == EOrionMovementState::Mantling;
		Ragdoll_ = State == EOrionMovementState::Ragdoll;
	}
};

USTRUCT(BlueprintType)
struct FOrionStance
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Character States")
	EOrionStance Stance = EOrionStance::Standing;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Character States")
	bool Standing_ = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Character States")
	bool Crouching_ = false;

public:
	FOrionStance()
	{
	}

	FOrionStance(const EOrionStance InitialStance) { *this = InitialStance; }

	const bool& Standing() const { return Standing_; }
	const bool& Crouching() const { return Crouching_; }

	operator EOrionStance() const { return Stance; }

	void operator=(const EOrionStance NewStance)
	{
		Stance = NewStance;
		Standing_ = Stance == EOrionStance::Standing;
		Crouching_ = Stance == EOrionStance::Crouching;
	}
};

USTRUCT(BlueprintType)
struct FOrionRotationMode
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Rotation System")
	EOrionRotationMode RotationMode = EOrionRotationMode::VelocityDirection;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Rotation System")
	bool VelocityDirection_ = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Rotation System")
	bool LookingDirection_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Rotation System")
	bool Aiming_ = false;

public:
	FOrionRotationMode()
	{
	}

	FOrionRotationMode(const EOrionRotationMode InitialRotationMode) { *this = InitialRotationMode; }

	const bool& VelocityDirection() const { return VelocityDirection_; }
	const bool& LookingDirection() const { return LookingDirection_; }
	const bool& Aiming() const { return Aiming_; }

	operator EOrionRotationMode() const { return RotationMode; }

	void operator=(const EOrionRotationMode NewRotationMode)
	{
		RotationMode = NewRotationMode;
		VelocityDirection_ = RotationMode == EOrionRotationMode::VelocityDirection;
		LookingDirection_ = RotationMode == EOrionRotationMode::LookingDirection;
		Aiming_ = RotationMode == EOrionRotationMode::Aiming;
	}
};

USTRUCT(BlueprintType)
struct FOrionMovementDirection
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Movement System")
	EOrionMovementDirection MovementDirection = EOrionMovementDirection::Forward;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Movement System")
	bool Forward_ = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Movement System")
	bool Right_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Movement System")
	bool Left_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Movement System")
	bool Backward_ = false;

public:
	FOrionMovementDirection()
	{
	}

	FOrionMovementDirection(const EOrionMovementDirection InitialMovementDirection)
	{
		*this = InitialMovementDirection;
	}

	const bool& Forward() const { return Forward_; }
	const bool& Right() const { return Right_; }
	const bool& Left() const { return Left_; }
	const bool& Backward() const { return Backward_; }

	operator EOrionMovementDirection() const { return MovementDirection; }

	void operator=(const EOrionMovementDirection NewMovementDirection)
	{
		MovementDirection = NewMovementDirection;
		Forward_ = MovementDirection == EOrionMovementDirection::Forward;
		Right_ = MovementDirection == EOrionMovementDirection::Right;
		Left_ = MovementDirection == EOrionMovementDirection::Left;
		Backward_ = MovementDirection == EOrionMovementDirection::Backward;
	}
};

USTRUCT(BlueprintType)
struct FOrionMovementAction
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Movement System")
	EOrionMovementAction Action = EOrionMovementAction::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Movement System")
	bool None_ = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Movement System")
	bool LowMantle_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Movement System")
	bool HighMantle_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Movement System")
	bool Rolling_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Movement System")
	bool GettingUp_ = false;

public:
	FOrionMovementAction()
	{
	}

	FOrionMovementAction(const EOrionMovementAction InitialAction) { *this = InitialAction; }

	const bool& None() const { return None_; }
	const bool& LowMantle() const { return LowMantle_; }
	const bool& HighMantle() const { return HighMantle_; }
	const bool& Rolling() const { return Rolling_; }
	const bool& GettingUp() const { return GettingUp_; }

	operator EOrionMovementAction() const { return Action; }

	void operator=(const EOrionMovementAction NewAction)
	{
		Action = NewAction;
		None_ = Action == EOrionMovementAction::None;
		LowMantle_ = Action == EOrionMovementAction::LowMantle;
		HighMantle_ = Action == EOrionMovementAction::HighMantle;
		Rolling_ = Action == EOrionMovementAction::Rolling;
		GettingUp_ = Action == EOrionMovementAction::GettingUp;
	}
};

USTRUCT(BlueprintType)
struct FOrionGait
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Movement System")
	EOrionGait Gait = EOrionGait::Walking;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Movement System")
	bool Walking_ = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Movement System")
	bool Running_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Movement System")
	bool Sprinting_ = false;

public:
	FOrionGait()
	{
	}

	FOrionGait(const EOrionGait InitialGait) { *this = InitialGait; }

	const bool& Walking() const { return Walking_; }
	const bool& Running() const { return Running_; }
	const bool& Sprinting() const { return Sprinting_; }

	operator EOrionGait() const { return Gait; }

	void operator=(const EOrionGait NewGait)
	{
		Gait = NewGait;
		Walking_ = Gait == EOrionGait::Walking;
		Running_ = Gait == EOrionGait::Running;
		Sprinting_ = Gait == EOrionGait::Sprinting;
	}
};

USTRUCT(BlueprintType)
struct FOrionOverlayState
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Character States")
	EOrionOverlayState State = EOrionOverlayState::Default;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Character States")
	bool Default_ = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Character States")
	bool Masculine_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Character States")
	bool Feminine_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Character States")
	bool Injured_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Character States")
	bool HandsTied_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Character States")
	bool Rifle_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Character States")
	bool PistolOneHanded_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Character States")
	bool PistolTwoHanded_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Character States")
	bool Bow_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Character States")
	bool Torch_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Character States")
	bool Binoculars_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Character States")
	bool Box_ = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Character States")
	bool Barrel_ = false;

public:
	FOrionOverlayState()
	{
	}

	FOrionOverlayState(const EOrionOverlayState InitialState) { *this = InitialState; }

	const bool& Default() const { return Default_; }
	const bool& Masculine() const { return Masculine_; }
	const bool& Feminine() const { return Feminine_; }
	const bool& Injured() const { return Injured_; }
	const bool& HandsTied() const { return HandsTied_; }
	const bool& Rifle() const { return Rifle_; }
	const bool& PistolOneHanded() const { return PistolOneHanded_; }
	const bool& PistolTwoHanded() const { return PistolTwoHanded_; }
	const bool& Bow() const { return Bow_; }
	const bool& Torch() const { return Torch_; }
	const bool& Binoculars() const { return Binoculars_; }
	const bool& Box() const { return Box_; }
	const bool& Barrel() const { return Barrel_; }

	operator EOrionOverlayState() const { return State; }

	void operator=(const EOrionOverlayState NewAction)
	{
		State = NewAction;
		Default_ = State == EOrionOverlayState::Default;
		Masculine_ = State == EOrionOverlayState::Masculine;
		Feminine_ = State == EOrionOverlayState::Feminine;
		Injured_ = State == EOrionOverlayState::Injured;
		HandsTied_ = State == EOrionOverlayState::HandsTied;
		Rifle_ = State == EOrionOverlayState::Rifle;
		PistolOneHanded_ = State == EOrionOverlayState::PistolOneHanded;
		PistolTwoHanded_ = State == EOrionOverlayState::PistolTwoHanded;
		Bow_ = State == EOrionOverlayState::Bow;
		Torch_ = State == EOrionOverlayState::Torch;
		Binoculars_ = State == EOrionOverlayState::Binoculars;
		Box_ = State == EOrionOverlayState::Box;
		Barrel_ = State == EOrionOverlayState::Barrel;
	}
};

USTRUCT(BlueprintType)
struct FOrionGroundedEntryState
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Breakfall System")
	EOrionGroundedEntryState State = EOrionGroundedEntryState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Breakfall System")
	bool None_ = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Orion|Breakfall System")
	bool Roll_ = false;

public:
	FOrionGroundedEntryState()
	{
	}

	FOrionGroundedEntryState(const EOrionGroundedEntryState InitialState) { *this = InitialState; }

	const bool& None() const { return None_; }
	const bool& Roll() const { return Roll_; }

	operator EOrionGroundedEntryState() const { return State; }

	void operator=(const EOrionGroundedEntryState NewState)
	{
		State = NewState;
		None_ = State == EOrionGroundedEntryState::None;
		Roll_ = State == EOrionGroundedEntryState::Roll;
	}
};
