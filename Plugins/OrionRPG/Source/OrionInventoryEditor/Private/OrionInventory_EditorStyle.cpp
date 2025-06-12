// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "OrionInventory_EditorStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FOrionInventory_EditorStyle::StyleInstance = nullptr;

void FOrionInventory_EditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FOrionInventory_EditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FOrionInventory_EditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("OrionInventory_EditorStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
const FVector2D Icon64x64(64.0f, 64.0f);

TSharedRef< FSlateStyleSet > FOrionInventory_EditorStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("OrionInventory_EditorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("OrionRPG")->GetBaseDir() / TEXT("Resources"));

	Style->Set("OrionInventory.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	Style->Set("OrionInventory.inventoryicon", new IMAGE_BRUSH_SVG(TEXT("inventoryicon"), Icon40x40));
	Style->Set("OrionInventory.inventoryicon.small", new IMAGE_BRUSH_SVG(TEXT("inventoryicon"), Icon20x20));
	return Style;
}

void FOrionInventory_EditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FOrionInventory_EditorStyle::Get()
{
	return *StyleInstance;
}
