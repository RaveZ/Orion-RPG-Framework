// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "DialogBuilder_EditorStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FDialogBuilder_EditorStyle::StyleInstance = nullptr;

void FDialogBuilder_EditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FDialogBuilder_EditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FDialogBuilder_EditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("DialogBuilder_EditorStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
const FVector2D Icon64x64(64.0f, 64.0f);

TSharedRef< FSlateStyleSet > FDialogBuilder_EditorStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("DialogBuilder_EditorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("QuestBuilder")->GetBaseDir() / TEXT("Resources"));

	Style->Set("DialogBuilder.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	Style->Set("DialogBuilder.dialogicon", new IMAGE_BRUSH_SVG(TEXT("dialogicon"), Icon40x40));
	Style->Set("DialogBuilder.dialogicon.small", new IMAGE_BRUSH_SVG(TEXT("dialogicon"), Icon20x20));
	return Style;
}

void FDialogBuilder_EditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FDialogBuilder_EditorStyle::Get()
{
	return *StyleInstance;
}
