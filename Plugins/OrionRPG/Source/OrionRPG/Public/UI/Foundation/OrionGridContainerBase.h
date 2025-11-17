// Copyright 2025 Ivan Chandra. All Rights Reserved.


#pragma once

#include "UI/OrionActivatableWidget.h"
#include "Types/SlateEnums.h"
#include "OrionGridContainerBase.generated.h"


// Grid container base class
UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class UOrionGridContainerBase : public UOrionActivatableWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	TArray<TObjectPtr<class UOrionGridSlot>> GridSlots;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	TArray<TObjectPtr<class UOrionGridItem>> GridItems;

	UPROPERTY(BlueprintReadWrite, Category = "UI", meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> GridPanel;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	TObjectPtr<class UOrionGridBase> CurrentFocusedGrid;
	
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	int32 CurrentFocusedGridIndex;


	UPROPERTY(BlueprintReadWrite, Category = "UI")
	int32 Rows;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	int32 Columns;

public:
	// UUserWidget interface
	virtual void NativePreConstruct() override;
	// End of UUserWidget interface

	UFUNCTION(BlueprintCallable, Category = "OrionRPG|UI")
	UWidget* DetermineGridNavigation(EUINavigation InNavigation);


public:
};
