// Fill out your copyright notice in the Description page of Project Settings.


#include "OrionUserWidget.h"

void UOrionUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}
