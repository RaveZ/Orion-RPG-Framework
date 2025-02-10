// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Aura/Public/AbilitySystem/AuraAbilitySystemComponent.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeAuraAbilitySystemComponent() {}

// Begin Cross Module References
AURA_API UClass* Z_Construct_UClass_UAuraAbilitySystemComponent();
AURA_API UClass* Z_Construct_UClass_UAuraAbilitySystemComponent_NoRegister();
GAMEPLAYABILITIES_API UClass* Z_Construct_UClass_UAbilitySystemComponent();
UPackage* Z_Construct_UPackage__Script_Aura();
// End Cross Module References

// Begin Class UAuraAbilitySystemComponent
void UAuraAbilitySystemComponent::StaticRegisterNativesUAuraAbilitySystemComponent()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UAuraAbilitySystemComponent);
UClass* Z_Construct_UClass_UAuraAbilitySystemComponent_NoRegister()
{
	return UAuraAbilitySystemComponent::StaticClass();
}
struct Z_Construct_UClass_UAuraAbilitySystemComponent_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "BlueprintSpawnableComponent", "" },
		{ "ClassGroupNames", "Custom" },
		{ "HideCategories", "Object LOD Lighting Transform Sockets TextureStreaming Object LOD Lighting Transform Sockets TextureStreaming" },
		{ "IncludePath", "AbilitySystem/AuraAbilitySystemComponent.h" },
		{ "ModuleRelativePath", "Public/AbilitySystem/AuraAbilitySystemComponent.h" },
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UAuraAbilitySystemComponent>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_UAuraAbilitySystemComponent_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UAbilitySystemComponent,
	(UObject* (*)())Z_Construct_UPackage__Script_Aura,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UAuraAbilitySystemComponent_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UAuraAbilitySystemComponent_Statics::ClassParams = {
	&UAuraAbilitySystemComponent::StaticClass,
	"Engine",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	nullptr,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	0,
	0,
	0x00B010A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UAuraAbilitySystemComponent_Statics::Class_MetaDataParams), Z_Construct_UClass_UAuraAbilitySystemComponent_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UAuraAbilitySystemComponent()
{
	if (!Z_Registration_Info_UClass_UAuraAbilitySystemComponent.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UAuraAbilitySystemComponent.OuterSingleton, Z_Construct_UClass_UAuraAbilitySystemComponent_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UAuraAbilitySystemComponent.OuterSingleton;
}
template<> AURA_API UClass* StaticClass<UAuraAbilitySystemComponent>()
{
	return UAuraAbilitySystemComponent::StaticClass();
}
DEFINE_VTABLE_PTR_HELPER_CTOR(UAuraAbilitySystemComponent);
UAuraAbilitySystemComponent::~UAuraAbilitySystemComponent() {}
// End Class UAuraAbilitySystemComponent

// Begin Registration
struct Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_AbilitySystem_AuraAbilitySystemComponent_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UAuraAbilitySystemComponent, UAuraAbilitySystemComponent::StaticClass, TEXT("UAuraAbilitySystemComponent"), &Z_Registration_Info_UClass_UAuraAbilitySystemComponent, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UAuraAbilitySystemComponent), 1565684029U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_AbilitySystem_AuraAbilitySystemComponent_h_3358148888(TEXT("/Script/Aura"),
	Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_AbilitySystem_AuraAbilitySystemComponent_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_AbilitySystem_AuraAbilitySystemComponent_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
