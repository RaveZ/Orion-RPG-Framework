// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "Actor/AuraEffectActor.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
class AActor;
class UPrimitiveComponent;
struct FHitResult;
#ifdef AURA_AuraEffectActor_generated_h
#error "AuraEffectActor.generated.h already included, missing '#pragma once' in AuraEffectActor.h"
#endif
#define AURA_AuraEffectActor_generated_h

#define FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_Actor_AuraEffectActor_h_14_RPC_WRAPPERS_NO_PURE_DECLS \
	DECLARE_FUNCTION(execEndOverlap); \
	DECLARE_FUNCTION(execOnOverlap);


#define FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_Actor_AuraEffectActor_h_14_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesAAuraEffectActor(); \
	friend struct Z_Construct_UClass_AAuraEffectActor_Statics; \
public: \
	DECLARE_CLASS(AAuraEffectActor, AActor, COMPILED_IN_FLAGS(0 | CLASS_Config), CASTCLASS_None, TEXT("/Script/Aura"), NO_API) \
	DECLARE_SERIALIZER(AAuraEffectActor)


#define FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_Actor_AuraEffectActor_h_14_ENHANCED_CONSTRUCTORS \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	AAuraEffectActor(AAuraEffectActor&&); \
	AAuraEffectActor(const AAuraEffectActor&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, AAuraEffectActor); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(AAuraEffectActor); \
	DEFINE_DEFAULT_CONSTRUCTOR_CALL(AAuraEffectActor) \
	NO_API virtual ~AAuraEffectActor();


#define FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_Actor_AuraEffectActor_h_11_PROLOG
#define FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_Actor_AuraEffectActor_h_14_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_Actor_AuraEffectActor_h_14_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_Actor_AuraEffectActor_h_14_INCLASS_NO_PURE_DECLS \
	FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_Actor_AuraEffectActor_h_14_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> AURA_API UClass* StaticClass<class AAuraEffectActor>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_Actor_AuraEffectActor_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
