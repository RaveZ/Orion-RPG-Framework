// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "OrionCombat_EditorStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FOrionCombat_EditorStyle::StyleInstance = nullptr;

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon24x24(24.0f, 24.0f);
const FVector2D Icon32x32(32.0f, 32.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
const FVector2D Icon64x64(64.0f, 64.0f);

void FOrionCombat_EditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FOrionCombat_EditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FOrionCombat_EditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("OrionCombatEditorStyle"));
	return StyleSetName;
}

TSharedRef< FSlateStyleSet > FOrionCombat_EditorStyle::Create()
{

	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("OrionRPG")->GetBaseDir() / TEXT("Resources"));

	//Thumbnails and icons
	Style->Set(FName(TEXT("ClassThumbnail.OrionCombat")), new IMAGE_BRUSH("OrionCombat", Icon64x64));
	return Style;
}

void FOrionCombat_EditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FOrionCombat_EditorStyle::Get()
{
	return *StyleInstance;
}
