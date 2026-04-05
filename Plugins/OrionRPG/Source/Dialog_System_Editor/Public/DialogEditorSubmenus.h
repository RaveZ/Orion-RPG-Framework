#pragma once

#include "CoreMinimal.h"

class UToolMenu;

namespace UE::DialogEditor
{
	void ExtendCameraSubmenu(FName InCameraOptionsSubmenuName);
	FToolMenuEntry CreateToolbarCameraSubmenu();
	FToolMenuEntry CreateToolbarViewportCameraModeEntry();
}