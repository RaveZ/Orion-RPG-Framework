// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Aura/Public/Player/AuraPlayerState.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeAuraPlayerState() {}

// Begin Cross Module References
AURA_API UClass* Z_Construct_UClass_AAuraPlayerState();
AURA_API UClass* Z_Construct_UClass_AAuraPlayerState_NoRegister();
ENGINE_API UClass* Z_Construct_UClass_APlayerState();
UPackage* Z_Construct_UPackage__Script_Aura();
// End Cross Module References

// Begin Class AAuraPlayerState
void AAuraPlayerState::StaticRegisterNativesAAuraPlayerState()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(AAuraPlayerState);
UClass* Z_Construct_UClass_AAuraPlayerState_NoRegister()
{
	return AAuraPlayerState::StaticClass();
}
struct Z_Construct_UClass_AAuraPlayerState_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \n */" },
#endif
		{ "HideCategories", "Input Movement Collision Rendering HLOD WorldPartition DataLayers Transformation" },
		{ "IncludePath", "Player/AuraPlayerState.h" },
		{ "ModuleRelativePath", "Public/Player/AuraPlayerState.h" },
		{ "ShowCategories", "Input|MouseInput Input|TouchInput" },
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AAuraPlayerState>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_AAuraPlayerState_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_APlayerState,
	(UObject* (*)())Z_Construct_UPackage__Script_Aura,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AAuraPlayerState_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_AAuraPlayerState_Statics::ClassParams = {
	&AAuraPlayerState::StaticClass,
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
	0x009002A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AAuraPlayerState_Statics::Class_MetaDataParams), Z_Construct_UClass_AAuraPlayerState_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_AAuraPlayerState()
{
	if (!Z_Registration_Info_UClass_AAuraPlayerState.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AAuraPlayerState.OuterSingleton, Z_Construct_UClass_AAuraPlayerState_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_AAuraPlayerState.OuterSingleton;
}
template<> AURA_API UClass* StaticClass<AAuraPlayerState>()
{
	return AAuraPlayerState::StaticClass();
}
DEFINE_VTABLE_PTR_HELPER_CTOR(AAuraPlayerState);
AAuraPlayerState::~AAuraPlayerState() {}
// End Class AAuraPlayerState

// Begin Registration
struct Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_Player_AuraPlayerState_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_AAuraPlayerState, AAuraPlayerState::StaticClass, TEXT("AAuraPlayerState"), &Z_Registration_Info_UClass_AAuraPlayerState, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AAuraPlayerState), 484793080U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_Player_AuraPlayerState_h_3814905844(TEXT("/Script/Aura"),
	Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_Player_AuraPlayerState_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_Player_AuraPlayerState_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
