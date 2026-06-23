// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OrionPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ORIONRPG_API AOrionPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AOrionPlayerController();
	
	//~APlayerController interface
	virtual void PreProcessInput(const float DeltaTime, const bool bGamePaused) override;
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	//~End of APlayerController interface
};
