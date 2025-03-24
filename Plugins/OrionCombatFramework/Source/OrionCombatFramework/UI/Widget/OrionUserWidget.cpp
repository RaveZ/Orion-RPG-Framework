// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "OrionUserWidget.h"

void UOrionUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}
