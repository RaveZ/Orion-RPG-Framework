// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Aura/Public/UI/HUD/AuraHUD.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeAuraHUD() {}

// Begin Cross Module References
AURA_API UClass* Z_Construct_UClass_AAuraHUD();
AURA_API UClass* Z_Construct_UClass_AAuraHUD_NoRegister();
AURA_API UClass* Z_Construct_UClass_UAuraUserWidget_NoRegister();
AURA_API UClass* Z_Construct_UClass_UOverlayWidgetController_NoRegister();
COREUOBJECT_API UClass* Z_Construct_UClass_UClass();
ENGINE_API UClass* Z_Construct_UClass_AHUD();
UPackage* Z_Construct_UPackage__Script_Aura();
// End Cross Module References

// Begin Class AAuraHUD
void AAuraHUD::StaticRegisterNativesAAuraHUD()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(AAuraHUD);
UClass* Z_Construct_UClass_AAuraHUD_NoRegister()
{
	return AAuraHUD::StaticClass();
}
struct Z_Construct_UClass_AAuraHUD_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \n */" },
#endif
		{ "HideCategories", "Rendering Actor Input Replication" },
		{ "IncludePath", "UI/HUD/AuraHUD.h" },
		{ "ModuleRelativePath", "Public/UI/HUD/AuraHUD.h" },
		{ "ShowCategories", "Input|MouseInput Input|TouchInput" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OverlayWidget_MetaData[] = {
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "Public/UI/HUD/AuraHUD.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OverlayWidgetClass_MetaData[] = {
		{ "Category", "AuraHUD" },
		{ "ModuleRelativePath", "Public/UI/HUD/AuraHUD.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OverlayWidgetController_MetaData[] = {
		{ "ModuleRelativePath", "Public/UI/HUD/AuraHUD.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OverlayWidgetControllerClass_MetaData[] = {
		{ "Category", "AuraHUD" },
		{ "ModuleRelativePath", "Public/UI/HUD/AuraHUD.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_OverlayWidget;
	static const UECodeGen_Private::FClassPropertyParams NewProp_OverlayWidgetClass;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_OverlayWidgetController;
	static const UECodeGen_Private::FClassPropertyParams NewProp_OverlayWidgetControllerClass;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AAuraHUD>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_AAuraHUD_Statics::NewProp_OverlayWidget = { "OverlayWidget", nullptr, (EPropertyFlags)0x0114000000080008, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AAuraHUD, OverlayWidget), Z_Construct_UClass_UAuraUserWidget_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_OverlayWidget_MetaData), NewProp_OverlayWidget_MetaData) };
const UECodeGen_Private::FClassPropertyParams Z_Construct_UClass_AAuraHUD_Statics::NewProp_OverlayWidgetClass = { "OverlayWidgetClass", nullptr, (EPropertyFlags)0x0044000000000001, UECodeGen_Private::EPropertyGenFlags::Class, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AAuraHUD, OverlayWidgetClass), Z_Construct_UClass_UClass, Z_Construct_UClass_UAuraUserWidget_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_OverlayWidgetClass_MetaData), NewProp_OverlayWidgetClass_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_AAuraHUD_Statics::NewProp_OverlayWidgetController = { "OverlayWidgetController", nullptr, (EPropertyFlags)0x0144000000000000, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AAuraHUD, OverlayWidgetController), Z_Construct_UClass_UOverlayWidgetController_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_OverlayWidgetController_MetaData), NewProp_OverlayWidgetController_MetaData) };
const UECodeGen_Private::FClassPropertyParams Z_Construct_UClass_AAuraHUD_Statics::NewProp_OverlayWidgetControllerClass = { "OverlayWidgetControllerClass", nullptr, (EPropertyFlags)0x0044000000000001, UECodeGen_Private::EPropertyGenFlags::Class, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AAuraHUD, OverlayWidgetControllerClass), Z_Construct_UClass_UClass, Z_Construct_UClass_UOverlayWidgetController_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_OverlayWidgetControllerClass_MetaData), NewProp_OverlayWidgetControllerClass_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_AAuraHUD_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AAuraHUD_Statics::NewProp_OverlayWidget,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AAuraHUD_Statics::NewProp_OverlayWidgetClass,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AAuraHUD_Statics::NewProp_OverlayWidgetController,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AAuraHUD_Statics::NewProp_OverlayWidgetControllerClass,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AAuraHUD_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_AAuraHUD_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AHUD,
	(UObject* (*)())Z_Construct_UPackage__Script_Aura,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AAuraHUD_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_AAuraHUD_Statics::ClassParams = {
	&AAuraHUD::StaticClass,
	"Game",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_AAuraHUD_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_AAuraHUD_Statics::PropPointers),
	0,
	0x009002ACu,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AAuraHUD_Statics::Class_MetaDataParams), Z_Construct_UClass_AAuraHUD_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_AAuraHUD()
{
	if (!Z_Registration_Info_UClass_AAuraHUD.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AAuraHUD.OuterSingleton, Z_Construct_UClass_AAuraHUD_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_AAuraHUD.OuterSingleton;
}
template<> AURA_API UClass* StaticClass<AAuraHUD>()
{
	return AAuraHUD::StaticClass();
}
AAuraHUD::AAuraHUD(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(AAuraHUD);
AAuraHUD::~AAuraHUD() {}
// End Class AAuraHUD

// Begin Registration
struct Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_UI_HUD_AuraHUD_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_AAuraHUD, AAuraHUD::StaticClass, TEXT("AAuraHUD"), &Z_Registration_Info_UClass_AAuraHUD, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AAuraHUD), 2298083488U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_UI_HUD_AuraHUD_h_121060924(TEXT("/Script/Aura"),
	Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_UI_HUD_AuraHUD_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_UI_HUD_AuraHUD_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
