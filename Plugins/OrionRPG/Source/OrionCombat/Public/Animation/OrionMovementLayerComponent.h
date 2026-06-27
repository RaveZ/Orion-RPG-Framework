// MovementLayerComponent.h
// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "Components/ActorComponent.h"
#include "OrionMovementLayerComponent.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_MovementLayer_Unarmed);

//////////////////////////////////////////////////////////////////////

// Movement Layer Struct
USTRUCT(BlueprintType)
struct FMovementLayer
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "MovementLayer"))
	FGameplayTag Tag;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> AnimInstance;
};

/**
 * Component to handle animation layer to character. Unarmed, sword, gun stance, etc.
 */
UCLASS(ClassGroup=(OrionCombat), meta=(BlueprintSpawnableComponent), AutoExpandCategories = "MovementLayer|Config")
class ORIONCOMBAT_API UOrionMovementLayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOrionMovementLayerComponent();
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MovementLayer")
	FGameplayTag CurrentOverlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovementLayer|Config", meta = (Categories = "MovementLayer"))
	FGameplayTag DefaultOverlay;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MovementLayer|Config")
	TArray<FMovementLayer> OverlayData;
    
public:
	UFUNCTION(BlueprintCallable, Category = "MovementLayer")
	void SetOverlay(UPARAM(meta = (Categories = "MovementLayer")) FGameplayTag Tag);
    
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