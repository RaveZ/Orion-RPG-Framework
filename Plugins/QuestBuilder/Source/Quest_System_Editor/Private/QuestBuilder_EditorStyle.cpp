// Copyright 2025 Ivan Chandra. All Rights Reserved.

#include "QuestBuilder_EditorStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FQuestBuilder_EditorStyle::StyleInstance = nullptr;

void FQuestBuilder_EditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FQuestBuilder_EditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FQuestBuilder_EditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("QuestBuilder_EditorStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
const FVector2D Icon64x64(64.0f, 64.0f);

TSharedRef< FSlateStyleSet > FQuestBuilder_EditorStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("QuestBuilder_EditorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("QuestBuilder")->GetBaseDir() / TEXT("Resources"));

	Style->Set("QuestBuilder.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	Style->Set("QuestBuilder.questicon", new IMAGE_BRUSH_SVG(TEXT("questicon"), Icon40x40));
	Style->Set("QuestBuilder.questicon.small", new IMAGE_BRUSH_SVG(TEXT("questicon"), Icon20x20));
	return Style;
}

void FQuestBuilder_EditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FQuestBuilder_EditorStyle::Get()
{
	return *StyleInstance;
}
