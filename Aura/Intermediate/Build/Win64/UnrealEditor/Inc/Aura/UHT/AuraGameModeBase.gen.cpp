// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Aura/Public/GameMode/AuraGameModeBase.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeAuraGameModeBase() {}

// Begin Cross Module References
AURA_API UClass* Z_Construct_UClass_AAuraGameModeBase();
AURA_API UClass* Z_Construct_UClass_AAuraGameModeBase_NoRegister();
ENGINE_API UClass* Z_Construct_UClass_AGameModeBase();
UPackage* Z_Construct_UPackage__Script_Aura();
// End Cross Module References

// Begin Class AAuraGameModeBase
void AAuraGameModeBase::StaticRegisterNativesAAuraGameModeBase()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(AAuraGameModeBase);
UClass* Z_Construct_UClass_AAuraGameModeBase_NoRegister()
{
	return AAuraGameModeBase::StaticClass();
}
struct Z_Construct_UClass_AAuraGameModeBase_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \n */" },
#endif
		{ "HideCategories", "Info Rendering MovementReplication Replication Actor Input Movement Collision Rendering HLOD WorldPartition DataLayers Transformation" },
		{ "IncludePath", "GameMode/AuraGameModeBase.h" },
		{ "ModuleRelativePath", "Public/GameMode/AuraGameModeBase.h" },
		{ "ShowCategories", "Input|MouseInput Input|TouchInput" },
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AAuraGameModeBase>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_AAuraGameModeBase_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AGameModeBase,
	(UObject* (*)())Z_Construct_UPackage__Script_Aura,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AAuraGameModeBase_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_AAuraGameModeBase_Statics::ClassParams = {
	&AAuraGameModeBase::StaticClass,
	"Game",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	nullptr,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	0,
	0,
	0x009002ACu,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AAuraGameModeBase_Statics::Class_MetaDataParams), Z_Construct_UClass_AAuraGameModeBase_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_AAuraGameModeBase()
{
	if (!Z_Registration_Info_UClass_AAuraGameModeBase.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AAuraGameModeBase.OuterSingleton, Z_Construct_UClass_AAuraGameModeBase_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_AAuraGameModeBase.OuterSingleton;
}
template<> AURA_API UClass* StaticClass<AAuraGameModeBase>()
{
	return AAuraGameModeBase::StaticClass();
}
AAuraGameModeBase::AAuraGameModeBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(AAuraGameModeBase);
AAuraGameModeBase::~AAuraGameModeBase() {}
// End Class AAuraGameModeBase

// Begin Registration
struct Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_GameMode_AuraGameModeBase_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_AAuraGameModeBase, AAuraGameModeBase::StaticClass, TEXT("AAuraGameModeBase"), &Z_Registration_Info_UClass_AAuraGameModeBase, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AAuraGameModeBase), 179199061U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_GameMode_AuraGameModeBase_h_3213757929(TEXT("/Script/Aura"),
	Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_GameMode_AuraGameModeBase_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_GameMode_AuraGameModeBase_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
