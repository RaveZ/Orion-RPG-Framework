// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Aura/Public/Interaction/EnemyInterface.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeEnemyInterface() {}

// Begin Cross Module References
AURA_API UClass* Z_Construct_UClass_UEnemyInterface();
AURA_API UClass* Z_Construct_UClass_UEnemyInterface_NoRegister();
COREUOBJECT_API UClass* Z_Construct_UClass_UInterface();
UPackage* Z_Construct_UPackage__Script_Aura();
// End Cross Module References

// Begin Interface UEnemyInterface
void UEnemyInterface::StaticRegisterNativesUEnemyInterface()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UEnemyInterface);
UClass* Z_Construct_UClass_UEnemyInterface_NoRegister()
{
	return UEnemyInterface::StaticClass();
}
struct Z_Construct_UClass_UEnemyInterface_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/Interaction/EnemyInterface.h" },
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<IEnemyInterface>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_UEnemyInterface_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UInterface,
	(UObject* (*)())Z_Construct_UPackage__Script_Aura,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UEnemyInterface_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UEnemyInterface_Statics::ClassParams = {
	&UEnemyInterface::StaticClass,
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
	0x000040A1u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UEnemyInterface_Statics::Class_MetaDataParams), Z_Construct_UClass_UEnemyInterface_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UEnemyInterface()
{
	if (!Z_Registration_Info_UClass_UEnemyInterface.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UEnemyInterface.OuterSingleton, Z_Construct_UClass_UEnemyInterface_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UEnemyInterface.OuterSingleton;
}
template<> AURA_API UClass* StaticClass<UEnemyInterface>()
{
	return UEnemyInterface::StaticClass();
}
UEnemyInterface::UEnemyInterface(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UEnemyInterface);
UEnemyInterface::~UEnemyInterface() {}
// End Interface UEnemyInterface

// Begin Registration
struct Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_Interaction_EnemyInterface_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UEnemyInterface, UEnemyInterface::StaticClass, TEXT("UEnemyInterface"), &Z_Registration_Info_UClass_UEnemyInterface, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UEnemyInterface), 868729569U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_Interaction_EnemyInterface_h_1153638215(TEXT("/Script/Aura"),
	Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_Interaction_EnemyInterface_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_Interaction_EnemyInterface_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
