// MovementLayerComponent.h
// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "MovementLayerComponent.generated.h"

//////////////////////////////////////////////////////////////////////

// Movement Layer Struct
USTRUCT(BlueprintType)
struct FMovementLayer
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Tag;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> AnimInstance;
};

/**
 * Component to handle animation layer to character. Unarmed, sword, gun stance, etc.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ORIONCOMBAT_API UMovementLayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMovementLayerComponent();
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MovementLayer|Runtime")
	FGameplayTag CurrentOverlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovementLayer|Config")
	FGameplayTag DefaultOverlay;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovementLayer|Config")
	TArray<FMovementLayer> OverlayData;
    
public:
	UFUNCTION(BlueprintCallable, Category = "MovementLayer")
	void SetOverlay(FGameplayTag Tag);
    
	UFUNCTION(BlueprintCallable, Category = "MovementLayer")
	void ClearOverlay();

	UFUNCTION(BlueprintPure, Category = "MovementLayer")
	FMovementLayer GetOverlayData(FGameplayTag Tag) const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	TSubclassOf<UAnimInstance> CurrentAnimInstanceClass;
    
	void LinkOverlayLayer(TSubclassOf<UAnimInstance> AnimInstanceClass);
	void UnlinkOverlayLayer();
};