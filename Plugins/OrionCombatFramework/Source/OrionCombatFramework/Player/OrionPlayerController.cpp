// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionPlayerController.h"


#include "OrionPlayerCameraManager.h"
#include "../Interaction/EnemyInterface.h"
#include "../AbilitySystem/OrionAbilitySystemComponent.h"
#include "../UI/Widget/DamageTextComponent.h"
#include "../OrionGameplayTags.h"
#include "../Components/OrionDebugComponent.h"
#include "../AI/OrionAIController.h"
#include "../Character/OrionCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AOrionPlayerController::AOrionPlayerController()
{
	bReplicates = true;
	PlayerCameraManagerClass = AOrionPlayerCameraManager::StaticClass();
}

void AOrionPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
}

void AOrionPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	
	CursorTrace();
}

void AOrionPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);
	PossessedCharacter = Cast<AOrionCharacterBase>(NewPawn);
	if (!IsRunningDedicatedServer())
	{
		// Servers want to setup camera only in listen servers.
		SetupCamera();
	}

	SetupInputs();

	if (!IsValid(PossessedCharacter)) return;

	UOrionDebugComponent* DebugComp = Cast<UOrionDebugComponent>(PossessedCharacter->GetComponentByClass(UOrionDebugComponent::StaticClass()));
	if (DebugComp)
	{
		DebugComp->OnPlayerControllerInitialized(this);
	}
}

void AOrionPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
	PossessedCharacter = Cast<AOrionCharacterBase>(GetPawn());
	SetupCamera();
	SetupInputs();

	if (!PossessedCharacter) return;

	UOrionDebugComponent* DebugComp = Cast<UOrionDebugComponent>(PossessedCharacter->GetComponentByClass(UOrionDebugComponent::StaticClass()));
	if (DebugComp)
	{
		DebugComp->OnPlayerControllerInitialized(this);
	}
}

void AOrionPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit)
{
	if (IsValid(TargetCharacter) && DamageTextComponentClass)
	{
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
		DamageText->RegisterComponent();
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DamageText->SetDamageText(DamageAmount, bBlockedHit, bCriticalHit);
	}
}

void AOrionPlayerController::CursorTrace()
{
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;
	ThisActor = CursorHit.GetActor();


	if (LastActor != ThisActor)
	{
		if (LastActor) LastActor->UnHighlightActor();
		if (ThisActor) ThisActor->HighlightActor();
	}
	
}

UOrionAbilitySystemComponent* AOrionPlayerController::GetASC()
{
	if (OrionAbilitySystemComponent == nullptr)
	{
		OrionAbilitySystemComponent = Cast<UOrionAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return OrionAbilitySystemComponent;
}

void AOrionPlayerController::SetupCamera()
{
	// Call "OnPossess" in Player Camera Manager when possessing a pawn
	AOrionPlayerCameraManager* CastedMgr = Cast<AOrionPlayerCameraManager>(PlayerCameraManager);
	if (PossessedCharacter && CastedMgr)
	{
		CastedMgr->OnPossess(PossessedCharacter);
	}
}

void AOrionPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->ClearActionEventBindings();
		EnhancedInputComponent->ClearActionValueBindings();
		EnhancedInputComponent->ClearDebugKeyBindings();

		BindActions(DefaultInputMappingContext);
		BindActions(DebugInputMappingContext); 
		BindAbilityActions(EnhancedInputComponent, InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);

	}
	else
	{
		UE_LOG(LogTemp, Fatal, TEXT("Orion Framework requires Enhanced Input System to be activated in project settings to function properly"));
	}

}

void AOrionPlayerController::BindActions(UInputMappingContext* Context)
{
	if (Context)
	{
		const TArray<FEnhancedActionKeyMapping>& Mappings = Context->GetMappings();
		UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
		if (EnhancedInputComponent)
		{
			// There may be more than one keymapping assigned to one action. So, first filter duplicate action entries to prevent multiple delegate bindings
			TSet<const UInputAction*> UniqueActions;
			for (const FEnhancedActionKeyMapping& Keymapping : Mappings)
			{
				UniqueActions.Add(Keymapping.Action);
			}
			//Function name must be the same for the Input Action name
			for (const UInputAction* UniqueAction : UniqueActions)
			{
				EnhancedInputComponent->BindAction(UniqueAction, ETriggerEvent::Triggered, Cast<UObject>(this), UniqueAction->GetFName());
			}
		}
	}
}

void AOrionPlayerController::SetupInputs()
{
	if (PossessedCharacter)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			FModifyContextOptions Options;
			Options.bForceImmediately = 1;
			Subsystem->AddMappingContext(DefaultInputMappingContext, 1, Options);
			UOrionDebugComponent* DebugComp = Cast<UOrionDebugComponent>(PossessedCharacter->GetComponentByClass(UOrionDebugComponent::StaticClass()));
			if (DebugComp)
			{
				// Do only if we have debug component
				Subsystem->AddMappingContext(DebugInputMappingContext, 0, Options);
			}
		}
	}
}

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
void AOrionPlayerController::BindAbilityActions(UEnhancedInputComponent* EnhancedInputComponent, const UOrionInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc)
{
	check(InputConfig);

	for (const FOrionInputAction& Action : InputConfig->AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			if (PressedFunc)
			{
				EnhancedInputComponent->BindAction(Action.InputAction, ETriggerEvent::Started, Object, PressedFunc, Action.InputTag);
			}

			if (ReleasedFunc)
			{
				EnhancedInputComponent->BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag);
			}

			if (HeldFunc)
			{
				EnhancedInputComponent->BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, HeldFunc, Action.InputTag);
			}
		}
	}
};


void AOrionPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (InputTag.MatchesTagExact(TAG_Input_BasicAttackAction))
	{
		bTargeting = ThisActor ? true : false;
	}
}

void AOrionPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (!InputTag.MatchesTagExact(TAG_Input_BasicAttackAction))
	{
		if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag);
		return;
	}

	if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag);

}

void AOrionPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (!InputTag.MatchesTagExact(TAG_Input_BasicAttackAction))
	{
		if (GetASC()) GetASC()->AbilityInputTagHeld(InputTag);
		return;
	}

	if (bTargeting)
	{
		if (GetASC()) GetASC()->AbilityInputTagHeld(InputTag);
	}
	else
	{
		FollowTime += GetWorld()->GetDeltaSeconds();
		if (CursorHit.bBlockingHit) CachedDestination = CursorHit.ImpactPoint;

		if (APawn* ControlledPawn = GetPawn())
		{
			const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
			ControlledPawn->AddMovementInput(WorldDirection);
		}
	}
}


void AOrionPlayerController::ForwardMovementAction(const FInputActionValue& Value)
{
	if (PossessedCharacter)
	{
		PossessedCharacter->ForwardMovementAction(Value.GetMagnitude());
	}
}

void AOrionPlayerController::RightMovementAction(const FInputActionValue& Value)
{
	if (PossessedCharacter)
	{
		PossessedCharacter->RightMovementAction(Value.GetMagnitude());
	}
}

void AOrionPlayerController::CameraUpAction(const FInputActionValue& Value)
{
	if (PossessedCharacter)
	{
		PossessedCharacter->CameraUpAction(Value.GetMagnitude());
	}
}

void AOrionPlayerController::CameraRightAction(const FInputActionValue& Value)
{
	if (PossessedCharacter)
	{
		PossessedCharacter->CameraRightAction(Value.GetMagnitude());
	}
}

void AOrionPlayerController::JumpAction(const FInputActionValue& Value)
{
	if (PossessedCharacter)
	{
		PossessedCharacter->JumpAction(Value.Get<bool>());
	}
}

void AOrionPlayerController::SprintAction(const FInputActionValue& Value)
{
	if (PossessedCharacter)
	{
		PossessedCharacter->SprintAction(Value.Get<bool>());
	}
}

void AOrionPlayerController::AimAction(const FInputActionValue& Value)
{
	if (PossessedCharacter)
	{
		PossessedCharacter->AimAction(Value.Get<bool>());
	}
}

void AOrionPlayerController::CameraTapAction(const FInputActionValue& Value)
{
	if (PossessedCharacter)
	{
		PossessedCharacter->CameraTapAction();
	}
}

void AOrionPlayerController::CameraHeldAction(const FInputActionValue& Value)
{
	if (PossessedCharacter)
	{
		PossessedCharacter->CameraHeldAction();
	}
}

void AOrionPlayerController::StanceAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && Value.Get<bool>())
	{
		PossessedCharacter->StanceAction();
	}
}

void AOrionPlayerController::WalkAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && Value.Get<bool>())
	{
		PossessedCharacter->WalkAction();
	}
}

void AOrionPlayerController::RagdollAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && Value.Get<bool>())
	{
		PossessedCharacter->RagdollAction();
	}
}

void AOrionPlayerController::VelocityDirectionAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && Value.Get<bool>())
	{
		PossessedCharacter->VelocityDirectionAction();
	}
}

void AOrionPlayerController::LookingDirectionAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && Value.Get<bool>())
	{
		PossessedCharacter->LookingDirectionAction();
	}
}

void AOrionPlayerController::DebugToggleHudAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && Value.Get<bool>())
	{
		UOrionDebugComponent* DebugComp = Cast<UOrionDebugComponent>(PossessedCharacter->GetComponentByClass(UOrionDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->ToggleHud();
		}
	}
}

void AOrionPlayerController::DebugToggleDebugViewAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && Value.Get<bool>())
	{
		UOrionDebugComponent* DebugComp = Cast<UOrionDebugComponent>(PossessedCharacter->GetComponentByClass(UOrionDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->ToggleDebugView();
		}
	}
}

void AOrionPlayerController::DebugToggleTracesAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && Value.Get<bool>())
	{
		UOrionDebugComponent* DebugComp = Cast<UOrionDebugComponent>(PossessedCharacter->GetComponentByClass(UOrionDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->ToggleTraces();
		}
	}
}

void AOrionPlayerController::DebugToggleShapesAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && Value.Get<bool>())
	{
		UOrionDebugComponent* DebugComp = Cast<UOrionDebugComponent>(PossessedCharacter->GetComponentByClass(UOrionDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->ToggleDebugShapes();
		}
	}
}

void AOrionPlayerController::DebugToggleLayerColorsAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && Value.Get<bool>())
	{
		UOrionDebugComponent* DebugComp = Cast<UOrionDebugComponent>(PossessedCharacter->GetComponentByClass(UOrionDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->ToggleLayerColors();
		}
	}
}

void AOrionPlayerController::DebugToggleCharacterInfoAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && Value.Get<bool>())
	{
		UOrionDebugComponent* DebugComp = Cast<UOrionDebugComponent>(PossessedCharacter->GetComponentByClass(UOrionDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->ToggleCharacterInfo();
		}
	}
}

void AOrionPlayerController::DebugToggleSlomoAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && Value.Get<bool>())
	{
		UOrionDebugComponent* DebugComp = Cast<UOrionDebugComponent>(PossessedCharacter->GetComponentByClass(UOrionDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->ToggleSlomo();
		}
	}
}

void AOrionPlayerController::DebugFocusedCharacterCycleAction(const FInputActionValue& Value)
{
	if (PossessedCharacter)
	{
		UOrionDebugComponent* DebugComp = Cast<UOrionDebugComponent>(PossessedCharacter->GetComponentByClass(UOrionDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->FocusedDebugCharacterCycle(Value.GetMagnitude() > 0);
		}
	}
}

void AOrionPlayerController::DebugToggleMeshAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && Value.Get<bool>())
	{
		UOrionDebugComponent* DebugComp = Cast<UOrionDebugComponent>(PossessedCharacter->GetComponentByClass(UOrionDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->ToggleDebugMesh();
		}
	}
}

void AOrionPlayerController::DebugOpenOverlayMenuAction(const FInputActionValue& Value)
{
	if (PossessedCharacter)
	{
		UOrionDebugComponent* DebugComp = Cast<UOrionDebugComponent>(PossessedCharacter->GetComponentByClass(UOrionDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->OpenOverlayMenu(Value.Get<bool>());
		}
	}
}

void AOrionPlayerController::DebugOverlayMenuCycleAction(const FInputActionValue& Value)
{
	if (PossessedCharacter)
	{
		UOrionDebugComponent* DebugComp = Cast<UOrionDebugComponent>(PossessedCharacter->GetComponentByClass(UOrionDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->OverlayMenuCycle(Value.GetMagnitude() > 0);
		}
	}
}
