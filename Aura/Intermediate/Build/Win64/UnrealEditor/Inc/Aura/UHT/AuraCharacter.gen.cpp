// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Aura/Public/Character/AuraCharacter.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeAuraCharacter() {}

// Begin Cross Module References
AURA_API UClass* Z_Construct_UClass_AAuraCharacter();
AURA_API UClass* Z_Construct_UClass_AAuraCharacter_NoRegister();
AURA_API UClass* Z_Construct_UClass_AAuraCharacterBase();
UPackage* Z_Construct_UPackage__Script_Aura();
// End Cross Module References

// Begin Class AAuraCharacter
void AAuraCharacter::StaticRegisterNativesAAuraCharacter()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(AAuraCharacter);
UClass* Z_Construct_UClass_AAuraCharacter_NoRegister()
{
	return AAuraCharacter::StaticClass();
}
struct Z_Construct_UClass_AAuraCharacter_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "HideCategories", "Navigation" },
		{ "IncludePath", "Character/AuraCharacter.h" },
		{ "ModuleRelativePath", "Public/Character/AuraCharacter.h" },
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AAuraCharacter>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_AAuraCharacter_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AAuraCharacterBase,
	(UObject* (*)())Z_Construct_UPackage__Script_Aura,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AAuraCharacter_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_AAuraCharacter_Statics::ClassParams = {
	&AAuraCharacter::StaticClass,
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
	0x009000A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AAuraCharacter_Statics::Class_MetaDataParams), Z_Construct_UClass_AAuraCharacter_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_AAuraCharacter()
{
	if (!Z_Registration_Info_UClass_AAuraCharacter.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AAuraCharacter.OuterSingleton, Z_Construct_UClass_AAuraCharacter_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_AAuraCharacter.OuterSingleton;
}
template<> AURA_API UClass* StaticClass<AAuraCharacter>()
{
	return AAuraCharacter::StaticClass();
}
DEFINE_VTABLE_PTR_HELPER_CTOR(AAuraCharacter);
AAuraCharacter::~AAuraCharacter() {}
// End Class AAuraCharacter

// Begin Registration
struct Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_Character_AuraCharacter_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_AAuraCharacter, AAuraCharacter::StaticClass, TEXT("AAuraCharacter"), &Z_Registration_Info_UClass_AAuraCharacter, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AAuraCharacter), 82608872U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_Character_AuraCharacter_h_3499171771(TEXT("/Script/Aura"),
	Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_Character_AuraCharacter_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_Character_AuraCharacter_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
