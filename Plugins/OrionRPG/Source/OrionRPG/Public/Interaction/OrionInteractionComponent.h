// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "Components/BoxComponent.h"
#include "NativeGameplayTags.h"
#include "OrionInteractionComponent.generated.h"


UENUM(BlueprintType)
enum class EOrionInteractionType : uint8
{
	/**Interaction trace from camera*/
	E_InteractionCamera		UMETA(DisplayName = "Camera"),
	/**Interaction trace from pawn*/
	E_InteractionPawn		UMETA(DisplayName = "Pawn"),
};

class UOrionInteractionTargetComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOrionInteractionSignature, UOrionInteractionTargetComponent*, InteractionTarget, FGameplayTag, InteractionTargetTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrionOnInteractingChanged, bool, bIsInteracting);
/**
 * Interaction component
 */
UCLASS(meta = (BlueprintSpawnableComponent), Blueprintable, HideDropdown, ClassGroup = "OrionRPG", DisplayName = "Orion Interaction")
class UOrionInteractionComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UOrionInteractionComponent();

	virtual void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//We cache the OwningController, we won't cache pawn as this might change
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<APlayerController> OwningController;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
    TEnumAsByte<ECollisionChannel> InteractionTraceChannel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	EOrionInteractionType InteractionType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction", meta = (DisplayName = "Debug Trace?"))
	bool bDebugTrace;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction", meta = (EditCondition = "InteractionType == EOrionInteractionType::E_InteractionPawn", HideEditConditionToggle = true, EditConditionHides = true))  
    FVector InteractionOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction", meta = (EditCondition = "InteractionType == EOrionInteractionType::E_InteractionPawn", HideEditConditionToggle = true, EditConditionHides = true))
	float InteractionRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction", meta = (EditCondition = "InteractionType == EOrionInteractionType::E_InteractionCamera", HideEditConditionToggle = true, EditConditionHides = true))
	float InteractionTraceDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	float InteractionTraceIntervalSecond;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TArray<FHitResult> InteractionhitResults;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<class UOrionInteractionTargetComponent> CurrentInteractionTarget;

	/*Index of the interaction list chosen by player*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Interaction")
	int32 InteractionIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Interaction")
	bool bIsInteractionEnabled;


	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOrionOnInteractingChanged OnInteractingStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOrionInteractionSignature OnInteractionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOrionInteractionSignature OnInteractionCanceled;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOrionInteractionSignature OnInteractionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOrionInteractionSignature OnInteractionUpdated;
	
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOrionInteractionSignature OnInteractionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOrionInteractionSignature OnInteractionBlocked;

protected:

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_bInteracting)
	bool bInteracting;

protected:
	UFUNCTION()
	void OnRep_bInteracting();
	/**
	 * Sets bInteracting Value and Calls Related On Rep
	 */

	UFUNCTION()
	void SetInteracting(bool bNewInteracting)
	{
		bInteracting = bNewInteracting;

		if (GetNetMode() != ENetMode::NM_DedicatedServer)
		{
			OnRep_bInteracting();
		}
	};

public:
	UFUNCTION(BlueprintPure, Category = "Interaction")
	virtual APawn* GetOwningPawn() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	virtual APlayerController* GetOwningController() const;

	UFUNCTION()
	virtual void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

public:
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void StartInteraction();

	/**
	 * RPC to Server To Start the Interaction
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StartInteraction();
	bool Server_StartInteraction_Validate() { return true; };

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractionEnabled(bool bEnabled);

	/**
	 * Tries to Cancel an Interaction on Authority Side of the Interaction
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void CancelInteraction();

	/**
	 * RPC to Server To Cancel the Interaction
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_CancelInteraction();
	bool Server_CancelInteraction_Validate() { return true; };

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void EndInteraction(UOrionInteractionTargetComponent* InteractionTarget, bool bCompleted);

	UFUNCTION()
	void RegisterNewInteractionTarget(UOrionInteractionTargetComponent* NewInteractionTarget);

	UFUNCTION()
	void DeRegisterInteractionTarget();

	/**
	 * Performs an Interaction Trace
	 *
	 * @param OutInteractionComponent - Out Interaction Component If Found
	 * @returns True If Interaction Exists
	 */
	UFUNCTION()
	FORCEINLINE bool TryGetInteraction(UOrionInteractionTargetComponent*& OutInteractionComponent)
	{
		OutInteractionComponent = GetInteractionTrace();
		return ValidateDirection(OutInteractionComponent);
	}

	UFUNCTION(BlueprintPure, Category = Interactor)
	bool IsLocalInteractor() const
	{
		const ENetMode NetMode = GetNetMode();
		const ENetRole Role = GetInteractorRole();

		if (NetMode == NM_Standalone)
		{
			return true;
		}

		if (NetMode == NM_Client && Role == ROLE_AutonomousProxy)
		{
			return true;
		}

		if (GetInteractorRemoteRole() != ROLE_AutonomousProxy && Role == ROLE_Authority)
		{
			return true;
		}

		return false;
	};

	/**
	 * Returns the Component Owner Role
	 */
	UFUNCTION(BlueprintPure, Category = Interactor)
	FORCEINLINE ENetRole GetInteractorRole() const
	{
		return IsValid(GetOwner()) ? GetOwner()->GetLocalRole() : ENetRole::ROLE_None;
	};

	/**
	 * Returns the Component Owner Remote Role
	 */
	UFUNCTION(BlueprintPure, Category = Interactor)
	FORCEINLINE ENetRole GetInteractorRemoteRole() const
	{
		return IsValid(GetOwner()) ? GetOwner()->GetRemoteRole() : ENetRole::ROLE_None;
	};

	/**
	 * Validates the Direction Towards Interaction Component
	 */
	UFUNCTION()
	bool ValidateDirection(const UOrionInteractionTargetComponent* InteractionTarget) const;

	/**
	 * Determines Whether this Interactor Instance Should Tick
	 *
	 * @return True If Local is Owner Or Instance is on Server
	 */
	UFUNCTION()
	FORCEINLINE bool ShouldTickInstance() const
	{
		return GetNetMode() == NM_Client && GetInteractorRole() != ROLE_AutonomousProxy ? false : true;
	}

private:
	UFUNCTION()
	void UpdateInteractionTrace(); 

	UFUNCTION()
	UOrionInteractionTargetComponent* GetInteractionTrace();
};
