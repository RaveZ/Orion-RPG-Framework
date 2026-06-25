// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "UI/OrionUIManagerSubsystem.h"
#include "CommonActivatableWidget.h"
#include "UI/OrionPrimaryGameLayout.h"
#include "UI/Foundation/OrionGridContainerBase.h"
#include "UI/Foundation/OrionGridBase.h"
#include "CommonInputSubsystem.h"
#include "Engine/GameInstance.h"	
#include "GameFramework/PlayerController.h"
#include "UObject/UObjectIterator.h"
#include "Engine/LocalPlayer.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Framework/Application/SlateApplication.h"

int32 UOrionUILibrary::InputSuspensions = 0;

//Gameplay Tags
UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Layer_Game, "UI.Layer.Game");
UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Layer_GameMenu, "UI.Layer.GameMenu");
UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Layer_Menu, "UI.Layer.Menu");
UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Layer_Modal , "UI.Layer.Modal");

UOrionUIManagerSubsystem::UOrionUIManagerSubsystem() : 
	OrionNavigationConfig(MakeShared<FOrionNavigationConfig>())
{
}

 
void UOrionUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FSlateApplication::Get().SetNavigationConfig(OrionNavigationConfig);
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UOrionUIManagerSubsystem::Tick));
}

void UOrionUIManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UOrionUIManagerSubsystem::Tick(float DeltaSeconds)
{
	// Get cursor position in desktop (absolute) coordinates
	FVector2D CursorPos = FSlateApplication::Get().GetCursorPos();

	// Try to get the mouse position in viewport coordinates (0..ViewportSize)
	FVector2D ViewportMousePos(0.0f, 0.0f);
	bool bHaveViewportMouse = false;
	if (UWorld* World = GetWorld())
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC)
		{
			float MX = 0.0f, MY = 0.0f;
			if (PC->GetMousePosition(MX, MY))
			{
				ViewportMousePos = FVector2D(MX, MY);
				bHaveViewportMouse = true;
			}
		}
	}

	if (DraggedItem)
	{
		bool bGridFollowMouse = true;
		if (CurrentFocusedGrid)
		{
			const FGeometry& GridGeometry = CurrentFocusedGrid->GetCachedGeometry();
			if (GridGeometry.IsUnderLocation(CursorPos))
			{
				bGridFollowMouse = false;
				FVector2D PixelPos;
				FVector2D ViewportPos;
				USlateBlueprintLibrary::LocalToViewport(this, GridGeometry, FVector2D(0.0f, 0.0f), PixelPos, ViewportPos);
				//DraggedItem->DragTargetPosition = PixelPos;
			}
		}

		if (bGridFollowMouse)
		{
			if (bHaveViewportMouse)
			{
				DraggedItem->DragTargetPosition = ViewportMousePos;
			}
			else
			{
				DraggedItem->DragTargetPosition = CursorPos;
			}
		}

	}

	return true;
}


bool UOrionUIManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance())
	{
		TArray<UClass*> ChildClasses;
		GetDerivedClasses(GetClass(), ChildClasses, false);

		// Only create an instance if there is no override implementation defined elsewhere
		return ChildClasses.Num() == 0;
	}

	return false;
}

void UOrionUIManagerSubsystem::CancelDragItem()
{
	if (DraggedItem)
	{
		OnCancelDragItem.Broadcast(DraggedItem);
		DraggedItem->RemoveFromParent();
		DraggedItem = nullptr;
	}

	if (CurrentFocusedGridContainer && CurrentFocusedGridContainer->CurrentFocusedGrid)
	{
		CurrentFocusedGridContainer->CurrentFocusedGrid->FocusGrid();
	}
	
}

FName UOrionUILibrary::SuspendInputForPlayer(APlayerController* PlayerController, FName SuspendReason)
{
	return SuspendInputForPlayer(PlayerController ? PlayerController->GetLocalPlayer() : nullptr, SuspendReason);
}

FName UOrionUILibrary::SuspendInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendReason)
{
	if (UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer))
	{
		InputSuspensions++;
		FName SuspendToken = SuspendReason;
		SuspendToken.SetNumber(InputSuspensions);

		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::MouseAndKeyboard, SuspendToken, true);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Gamepad, SuspendToken, true);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Touch, SuspendToken, true);

		return SuspendToken;
	}

	return NAME_None;
}

void UOrionUILibrary::ResumeInputForPlayer(APlayerController* PlayerController, FName SuspendToken)
{
	ResumeInputForPlayer(PlayerController ? PlayerController->GetLocalPlayer() : nullptr, SuspendToken);
}

void UOrionUILibrary::ResumeInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendToken)
{
	if (SuspendToken == NAME_None)
	{
		return;
	}

	if (UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer))
	{
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::MouseAndKeyboard, SuspendToken, false);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Gamepad, SuspendToken, false);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Touch, SuspendToken, false);
	}
}

UOrionUIManagerSubsystem* UOrionUILibrary::GetOrionUISubsystem(const UObject* WorldContextObject)
{
	UWorld* World = WorldContextObject->GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	if (GameInstance)
	{
		return GameInstance->GetSubsystem<UOrionUIManagerSubsystem>();
	}
	return nullptr;
}



UCommonActivatableWidgetContainerBase* UOrionUILibrary::GetLayerWidget(const UObject* WorldContextObject, FGameplayTag LayerName)
{
	UWorld* World = WorldContextObject->GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	if (GameInstance)
	{
		UOrionUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UOrionUIManagerSubsystem>();
		if (UIManager && UIManager->PrimaryLayout)
		{
			return UIManager->PrimaryLayout->GetLayerWidget(LayerName);
		}
	}
		
	
	return nullptr;
    
}

UCommonActivatableWidget* UOrionUILibrary::GetActiveWidgetFromLayer(const UObject* WorldContextObject, FGameplayTag LayerName)
{
	UCommonActivatableWidgetContainerBase* WidgetContainer = GetLayerWidget(WorldContextObject, LayerName);
	if (WidgetContainer)
	{
		return WidgetContainer->GetActiveWidget();
	}
	return nullptr;
}

UWidget* UOrionUILibrary::GetFocusedWidget()
{
	TSharedPtr<SWidget> FocusedSlateWidget = FSlateApplication::Get().GetUserFocusedWidget(0);
	if (!FocusedSlateWidget.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("No focused Slate widget found"));
		return nullptr;
	}
	for (TObjectIterator<UWidget> Itr; Itr; ++Itr)
	{
		UWidget* CandidateUMGWidget = *Itr;
		if (CandidateUMGWidget->GetCachedWidget() == FocusedSlateWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("Focused UMG widget found: %s"), *CandidateUMGWidget->GetName());
			return CandidateUMGWidget;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("No focused UMG widget found"));
	return nullptr;
}