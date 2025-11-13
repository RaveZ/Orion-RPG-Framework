// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "UI/Foundation/OrionButtonBase.h"
#include "OrionGridBase.generated.h"

class UObject;
class UOrionInventoryItemInstance;
struct FFrame;

UCLASS(Abstract, BlueprintType, Blueprintable)
class UOrionGridBase : public UOrionButtonBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	int32 GridIndex;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	float GridSize;

	
public:
	// UUserWidget interface
	virtual void NativePreConstruct() override;
	// End of UUserWidget interface

	
};


UCLASS(Abstract, BlueprintType, Blueprintable)
class UOrionGridSlot : public UOrionGridBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	TObjectPtr<UOrionGridItem> ContainedGridItem;

public:
	// UUserWidget interface
	virtual void NativePreConstruct() override;
	// End of UUserWidget interface
};


UCLASS(Abstract, BlueprintType, Blueprintable)
class UOrionGridItem : public UOrionGridBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	TArray<TObjectPtr<UOrionGridSlot>> OccupiedSlots;
	
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	TObjectPtr<UOrionInventoryItemInstance> ItemInstance;

public:
	// UUserWidget interface
	virtual void NativePreConstruct() override;
	// End of UUserWidget interface

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="OrionRPG|UI")
	void SetItemInstance(UOrionInventoryItemInstance* InItemInstance, bool bUpdateItemIndex);

	void SetItemInstance_Implementation(UOrionInventoryItemInstance* InItemInstance, bool bUpdateItemIndex = true);

};
