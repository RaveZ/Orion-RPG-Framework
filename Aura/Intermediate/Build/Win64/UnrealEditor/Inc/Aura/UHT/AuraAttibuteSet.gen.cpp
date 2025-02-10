// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Aura/Public/AbilitySystem/AuraAttibuteSet.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeAuraAttibuteSet() {}

// Begin Cross Module References
AURA_API UClass* Z_Construct_UClass_UAuraAttibuteSet();
AURA_API UClass* Z_Construct_UClass_UAuraAttibuteSet_NoRegister();
GAMEPLAYABILITIES_API UClass* Z_Construct_UClass_UAttributeSet();
UPackage* Z_Construct_UPackage__Script_Aura();
// End Cross Module References

// Begin Class UAuraAttibuteSet
void UAuraAttibuteSet::StaticRegisterNativesUAuraAttibuteSet()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UAuraAttibuteSet);
UClass* Z_Construct_UClass_UAuraAttibuteSet_NoRegister()
{
	return UAuraAttibuteSet::StaticClass();
}
struct Z_Construct_UClass_UAuraAttibuteSet_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \n */" },
#endif
		{ "IncludePath", "AbilitySystem/AuraAttibuteSet.h" },
		{ "ModuleRelativePath", "Public/AbilitySystem/AuraAttibuteSet.h" },
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UAuraAttibuteSet>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_UAuraAttibuteSet_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UAttributeSet,
	(UObject* (*)())Z_Construct_UPackage__Script_Aura,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UAuraAttibuteSet_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UAuraAttibuteSet_Statics::ClassParams = {
	&UAuraAttibuteSet::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	nullptr,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	0,
	0,
	0x003000A0u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UAuraAttibuteSet_Statics::Class_MetaDataParams), Z_Construct_UClass_UAuraAttibuteSet_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UAuraAttibuteSet()
{
	if (!Z_Registration_Info_UClass_UAuraAttibuteSet.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UAuraAttibuteSet.OuterSingleton, Z_Construct_UClass_UAuraAttibuteSet_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UAuraAttibuteSet.OuterSingleton;
}
template<> AURA_API UClass* StaticClass<UAuraAttibuteSet>()
{
	return UAuraAttibuteSet::StaticClass();
}
UAuraAttibuteSet::UAuraAttibuteSet(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UAuraAttibuteSet);
UAuraAttibuteSet::~UAuraAttibuteSet() {}
// End Class UAuraAttibuteSet

// Begin Registration
struct Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_AbilitySystem_AuraAttibuteSet_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UAuraAttibuteSet, UAuraAttibuteSet::StaticClass, TEXT("UAuraAttibuteSet"), &Z_Registration_Info_UClass_UAuraAttibuteSet, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UAuraAttibuteSet), 1060808168U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_AbilitySystem_AuraAttibuteSet_h_1523290713(TEXT("/Script/Aura"),
	Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_AbilitySystem_AuraAttibuteSet_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_AbilitySystem_AuraAttibuteSet_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
