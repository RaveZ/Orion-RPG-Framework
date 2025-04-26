// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OrionCharacterEnumLibrary.generated.h"

/* Returns the enumeration index. */
template <typename Enumeration>
static FORCEINLINE int32 GetEnumerationIndex(const Enumeration InValue)
{
	return StaticEnum<Enumeration>()->GetIndexByValue(static_cast<int64>(InValue));
}

/* Returns the enumeration value as string. */
template <typename Enumeration>
static FORCEINLINE FString GetEnumerationToString(const Enumeration InValue)
{
	return StaticEnum<Enumeration>()->GetNameStringByValue(static_cast<int64>(InValue));
}

/**
 * Character gait state. Note: Also edit related struct in OrionStructEnumLibrary if you add new enums
 */
UENUM(BlueprintType, meta = (ScriptName = "Orion_Gait"))
enum class EOrionGait : uint8
{
	Walking,
	Running,
	Sprinting
};

/**
 * Character movement action state. Note: Also edit related struct in OrionStructEnumLibrary if you add new enums
 */
UENUM(BlueprintType, meta = (ScriptName = "Orion_MovementAction"))
enum class EOrionMovementAction : uint8
{
	None,
	LowMantle,
	HighMantle,
	Rolling,
	GettingUp
};

/**
 * Character movement state. Note: Also edit related struct in OrionStructEnumLibrary if you add new enums
 */
UENUM(BlueprintType, meta = (ScriptName = "Orion_MovementState"))
enum class EOrionMovementState : uint8
{
	None,
	Grounded,
	InAir,
	Mantling,
	Ragdoll
};

/**
 * Character overlay state. Note: Also edit related struct in OrionStructEnumLibrary if you add new enums
 */
UENUM(BlueprintType, meta = (ScriptName = "Orion_OverlayState"))
enum class EOrionOverlayState : uint8
{
	Default,
	Masculine,
	Feminine,
	Injured,
	HandsTied,
	Rifle,
	PistolOneHanded,
	PistolTwoHanded,
	Bow,
	Torch,
	Binoculars,
	Box,
	Barrel
};

/**
 * Character rotation mode. Note: Also edit related struct in OrionStructEnumLibrary if you add new enums
 */
UENUM(BlueprintType, meta = (ScriptName="Orion_RotationMode"))
enum class EOrionRotationMode : uint8
{
	VelocityDirection,
	LookingDirection,
	Aiming
};

/**
 * Character stance. Note: Also edit related struct in OrionStructEnumLibrary if you add new enums
 */
UENUM(BlueprintType, meta = (ScriptName = "Orion_Stance"))
enum class EOrionStance : uint8
{
	Standing,
	Crouching
};

/**
 * Character view mode. Note: Also edit related struct in OrionStructEnumLibrary if you add new enums
 */
UENUM(BlueprintType, meta = (ScriptName = "Orion_ViewMode"))
enum class EOrionViewMode : uint8
{
	ThirdPerson,
	FirstPerson
};

UENUM(BlueprintType, meta = (ScriptName = "Orion_AnimFeatureExample"))
enum class EOrionAnimFeatureExample : uint8
{
	StrideBlending,
	AdditiveBlending,
	SprintImpulse
};

UENUM(BlueprintType, meta = (ScriptName = "Orion_FootstepType"))
enum class EOrionFootstepType : uint8
{
	Step,
	WalkRun,
	Jump,
	Land
};

UENUM(BlueprintType, meta = (ScriptName = "Orion_GroundedEntryState"))
enum class EOrionGroundedEntryState : uint8
{
	None,
	Roll
};

UENUM(BlueprintType, meta = (ScriptName = "Orion_HipsDirection"))
enum class EOrionHipsDirection : uint8
{
	F,
	B,
	RF,
	RB,
	LF,
	LB
};

UENUM(BlueprintType, meta = (ScriptName = "Orion_MantleType"))
enum class EOrionMantleType : uint8
{
	HighMantle,
	LowMantle,
	FallingCatch
};

UENUM(BlueprintType, meta = (ScriptName = "Orion_MovementDirection"))
enum class EOrionMovementDirection : uint8
{
	Forward,
	Right,
	Left,
	Backward
};

UENUM(BlueprintType, meta = (ScriptName = "Orion_SpawnType"))
enum class EOrionSpawnType : uint8
{
	Location,
	Attached
};
