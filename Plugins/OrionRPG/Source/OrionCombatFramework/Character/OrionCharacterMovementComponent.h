// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Library/OrionCharacterStructLibrary.h"

#include "OrionCharacterMovementComponent.generated.h"

/**
 * Authoritative networked Character Movement
 */
UCLASS()
class ORIONCOMBATFRAMEWORK_API UOrionCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()

	class ORIONCOMBATFRAMEWORK_API FSavedMove_My : public FSavedMove_Character
	{
	public:

		typedef FSavedMove_Character Super;

		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel,
			class FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(class ACharacter* Character) override;

		// Walk Speed Update
		uint8 bSavedRedialogMovementSettingsChange : 1;
		EOrionGait SavedAllowedGait = EOrionGait::Walking;
	};

	class ORIONCOMBATFRAMEWORK_API FNetworkPredictionData_Client_My : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_My(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override;
	};

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity) override;

	// Movement Settings Override
	virtual void PhysWalking(float deltaTime, int32 Iterations) override;
	virtual float GetMaxAcceleration() const override;
	virtual float GetMaxBrakingDeceleration() const override;

	// Movement Settings Variables
	UPROPERTY()
	uint8 bRedialogMovementSettingsChange = 1;

	UPROPERTY()
	EOrionGait AllowedGait = EOrionGait::Walking;

	UPROPERTY(BlueprintReadOnly, Category = "Orion|Movement System")
	FOrionMovementSettings CurrentMovementSettings;

	// Set Movement Curve (Called in every instance)
	float GetMappedSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Movement Settings")
	void SetMovementSettings(FOrionMovementSettings NewMovementSettings);

	// Set Max Walking Speed (Called from the owning client)
	UFUNCTION(BlueprintCallable, Category = "Movement Settings")
	void SetAllowedGait(EOrionGait NewAllowedGait);

	UFUNCTION(Reliable, Server, Category = "Movement Settings")
	void Server_SetAllowedGait(EOrionGait NewAllowedGait);
};
