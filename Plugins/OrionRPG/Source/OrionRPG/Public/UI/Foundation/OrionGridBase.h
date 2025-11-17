// Copyright 2025 Ivan Chandra. All Rights Reserved.

#pragma once

#include "UI/Foundation/OrionButtonBase.h"
#include "OrionGridBase.generated.h"

class UObject;
class UOrionInventoryItemInstance;
struct FFrame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGridItemRemoved);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGridItemChanged, UOrionInventoryItemInstance*, ItemInstance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAddItemToSlot, UOrionInventoryItemInstance*, ItemInstance, int32, GridIndex);


//////////////////////////////////////////////////////////////////////

// Orion Grid Base
UCLASS(Abstract, BlueprintType, Blueprintable)
class UOrionGridBase : public UOrionButtonBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	TObjectPtr<class UOrionGridContainerBase> OwningGridContainer;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	int32 GridIndex;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	float GridSize;

	
public:
	// UUserWidget interface
	virtual void NativePreConstruct() override;
	// End of UUserWidget interface

	UFUNCTION(BlueprintCallable, Category = "OrionRPG|UI")
	virtual void InitializeGrid(class UOrionGridContainerBase* InGridContainer);

	UFUNCTION(BlueprintCallable, Category = "OrionRPG|UI")
	int32 GetIndexFromPosition(int32 PosX, int32 PosY, int32 Columns);
	
};


//////////////////////////////////////////////////////////////////////

// Orion Grid Slot
UCLASS(Abstract, BlueprintType, Blueprintable)
class UOrionGridSlot : public UOrionGridBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	TObjectPtr<UOrionGridItem> GridItem;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "UI")
	FOnAddItemToSlot OnAddItemToSlot;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "OrionRPG|UI")
	void SetGridItem(UOrionGridItem* InGridItem);

	void SetGridItem_Implementation(UOrionGridItem* InGridItem);


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "OrionRPG|UI")
	void HandleGridItemRemoved();

	void HandleGridItemRemoved_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "OrionRPG|UI")
	void HandleGridItemChanged(UOrionInventoryItemInstance* InItemInstance);

	void HandleGridItemChanged_Implementation(UOrionInventoryItemInstance* InItemInstance);

public:
	// UUserWidget interface
	virtual void NativePreConstruct() override;
	// End of UUserWidget interface

	virtual void InitializeGrid(class UOrionGridContainerBase* InGridContainer) override;
};


//////////////////////////////////////////////////////////////////////

// Orion Grid Item
UCLASS(Abstract, BlueprintType, Blueprintable)
class UOrionGridItem : public UOrionGridBase
{
	GENERATED_BODY()
public:
	
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	TObjectPtr<UOrionInventoryItemInstance> ItemInstance;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	FIntPoint ItemDimension;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	FVector2D DragTargetPosition;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "UI")
	FOnGridItemRemoved OnItemRemoved;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "UI")
	FOnGridItemChanged OnItemChanged;

public:
	// UUserWidget interface
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	// End of UUserWidget interface

	virtual void InitializeGrid(class UOrionGridContainerBase* InGridContainer) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="OrionRPG|UI")
	void SetItemInstance(UOrionInventoryItemInstance* InItemInstance, bool bUpdateItemIndex);

	void SetItemInstance_Implementation(UOrionInventoryItemInstance* InItemInstance, bool bUpdateItemIndex = true);

	UFUNCTION(BlueprintCallable, Category = "OrionRPG|UI")
	void UpdateItemSize(UOrionInventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, Category = "OrionRPG|UI")
	bool IsValidSlot(UOrionGridSlot* InOtherSlot);

};
