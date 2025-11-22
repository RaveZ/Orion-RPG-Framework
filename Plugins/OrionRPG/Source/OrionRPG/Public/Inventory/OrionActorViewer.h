// Copyright 2025 Ivan Chandra. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OrionActorViewer.generated.h"

UCLASS(Blueprintable, BlueprintType, Abstract)
class ORIONRPG_API AOrionActorViewer : public AActor
{
    GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	float CurrentFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	float MinFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	float MaxFOV;
public:
	AOrionActorViewer();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	void Zoom(float Delta);

	void Zoom_Implementation(float Delta);


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	void Rotate(float DeltaX, float DeltaY, float Sensitivity);

	void Rotate_Implementation(float DeltaX, float DeltaY, float Sensitivity);
};