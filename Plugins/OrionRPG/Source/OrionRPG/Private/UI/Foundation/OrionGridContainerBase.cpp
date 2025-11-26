// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "UI/Foundation/OrionGridContainerBase.h"
#include "UI/OrionUIManagerSubsystem.h"
#include "UI/Foundation/OrionGridBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OrionGridContainerBase)

#define LOCTEXT_NAMESPACE "OrionRPG"

void UOrionGridContainerBase::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UOrionGridContainerBase::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	UOrionUIManagerSubsystem* UISubsystem = UOrionUILibrary::GetOrionUISubsystem(this);
	UISubsystem->CurrentFocusedGridContainer = this;
}

UWidget* UOrionGridContainerBase::DetermineGridNavigation(EUINavigation InNavigation)
{
	UOrionUIManagerSubsystem* UIManager = UOrionUILibrary::GetOrionUISubsystem(this);
	UOrionGridItem* DraggedItem = UIManager ? UIManager->DraggedItem : nullptr;
	int32 NextGridIndex = CurrentFocusedGridIndex;
	UOrionGridItem* CurrentGridItem = GridSlots[CurrentFocusedGridIndex] ? GridSlots[CurrentFocusedGridIndex]->GridItem : nullptr;
	switch (InNavigation)
	{
		case EUINavigation::Left:
			NextGridIndex -= 1;
			if (!GridSlots.IsValidIndex(NextGridIndex)) return nullptr;
			if (CurrentFocusedGridIndex % Columns == 0) return nullptr;
			break;
		case EUINavigation::Right:
			NextGridIndex += DraggedItem ? 1 : CurrentGridItem ? CurrentGridItem->ItemDimension.X : 1;
			if (!GridSlots.IsValidIndex(NextGridIndex)) return nullptr;
			if (CurrentFocusedGridIndex % Columns == Columns-1) return nullptr;
			break;
		case EUINavigation::Up:
			NextGridIndex -= Columns;
			if (!GridSlots.IsValidIndex(NextGridIndex)) return nullptr;
			break;
		case EUINavigation::Down:
			NextGridIndex += DraggedItem ? Columns : CurrentGridItem ? CurrentGridItem->ItemDimension.Y * Columns : Columns;
			if (!GridSlots.IsValidIndex(NextGridIndex)) return nullptr;
			break;
	}

	return Cast<UWidget>(GridSlots[NextGridIndex]);
}

void UOrionGridContainerBase::CreateDragItem_Implementation(UOrionInventoryItemInstance* InItemInstance, int32 FromIndex)
{
}

#undef LOCTEXT_NAMESPACE

