// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Aura/Public/UI/WidgetController/OverlayWidgetController.h"
#include "Runtime/GameplayTags/Classes/GameplayTagContainer.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeOverlayWidgetController() {}

// Begin Cross Module References
AURA_API UClass* Z_Construct_UClass_UAuraUserWidget_NoRegister();
AURA_API UClass* Z_Construct_UClass_UAuraWidgetController();
AURA_API UClass* Z_Construct_UClass_UOverlayWidgetController();
AURA_API UClass* Z_Construct_UClass_UOverlayWidgetController_NoRegister();
AURA_API UFunction* Z_Construct_UDelegateFunction_Aura_OnHealthChangedSignature__DelegateSignature();
AURA_API UFunction* Z_Construct_UDelegateFunction_Aura_OnManaChangedSignature__DelegateSignature();
AURA_API UFunction* Z_Construct_UDelegateFunction_Aura_OnMaxHealthChangedSignature__DelegateSignature();
AURA_API UFunction* Z_Construct_UDelegateFunction_Aura_OnMaxManaChangedSignature__DelegateSignature();
AURA_API UScriptStruct* Z_Construct_UScriptStruct_FUIWidgetRow();
COREUOBJECT_API UClass* Z_Construct_UClass_UClass();
ENGINE_API UClass* Z_Construct_UClass_UDataTable_NoRegister();
ENGINE_API UClass* Z_Construct_UClass_UTexture2D_NoRegister();
ENGINE_API UScriptStruct* Z_Construct_UScriptStruct_FTableRowBase();
GAMEPLAYTAGS_API UScriptStruct* Z_Construct_UScriptStruct_FGameplayTag();
UPackage* Z_Construct_UPackage__Script_Aura();
// End Cross Module References

// Begin Delegate FOnHealthChangedSignature
struct Z_Construct_UDelegateFunction_Aura_OnHealthChangedSignature__DelegateSignature_Statics
{
	struct _Script_Aura_eventOnHealthChangedSignature_Parms
	{
		float NewHealth;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/UI/WidgetController/OverlayWidgetController.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FFloatPropertyParams NewProp_NewHealth;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UDelegateFunction_Aura_OnHealthChangedSignature__DelegateSignature_Statics::NewProp_NewHealth = { "NewHealth", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(_Script_Aura_eventOnHealthChangedSignature_Parms, NewHealth), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UDelegateFunction_Aura_OnHealthChangedSignature__DelegateSignature_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UDelegateFunction_Aura_OnHealthChangedSignature__DelegateSignature_Statics::NewProp_NewHealth,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UDelegateFunction_Aura_OnHealthChangedSignature__DelegateSignature_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UDelegateFunction_Aura_OnHealthChangedSignature__DelegateSignature_Statics::FuncParams = { (UObject*(*)())Z_Construct_UPackage__Script_Aura, nullptr, "OnHealthChangedSignature__DelegateSignature", nullptr, nullptr, Z_Construct_UDelegateFunction_Aura_OnHealthChangedSignature__DelegateSignature_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UDelegateFunction_Aura_OnHealthChangedSignature__DelegateSignature_Statics::PropPointers), sizeof(Z_Construct_UDelegateFunction_Aura_OnHealthChangedSignature__DelegateSignature_Statics::_Script_Aura_eventOnHealthChangedSignature_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x00130000, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UDelegateFunction_Aura_OnHealthChangedSignature__DelegateSignature_Statics::Function_MetaDataParams), Z_Construct_UDelegateFunction_Aura_OnHealthChangedSignature__DelegateSignature_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UDelegateFunction_Aura_OnHealthChangedSignature__DelegateSignature_Statics::_Script_Aura_eventOnHealthChangedSignature_Parms) < MAX_uint16);
UFunction* Z_Construct_UDelegateFunction_Aura_OnHealthChangedSignature__DelegateSignature()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UDelegateFunction_Aura_OnHealthChangedSignature__DelegateSignature_Statics::FuncParams);
	}
	return ReturnFunction;
}
void FOnHealthChangedSignature_DelegateWrapper(const FMulticastScriptDelegate& OnHealthChangedSignature, float NewHealth)
{
	struct _Script_Aura_eventOnHealthChangedSignature_Parms
	{
		float NewHealth;
	};
	_Script_Aura_eventOnHealthChangedSignature_Parms Parms;
	Parms.NewHealth=NewHealth;
	OnHealthChangedSignature.ProcessMulticastDelegate<UObject>(&Parms);
}
// End Delegate FOnHealthChangedSignature

// Begin Delegate FOnMaxHealthChangedSignature
struct Z_Construct_UDelegateFunction_Aura_OnMaxHealthChangedSignature__DelegateSignature_Statics
{
	struct _Script_Aura_eventOnMaxHealthChangedSignature_Parms
	{
		float NewMaxHealth;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/UI/WidgetController/OverlayWidgetController.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FFloatPropertyParams NewProp_NewMaxHealth;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UDelegateFunction_Aura_OnMaxHealthChangedSignature__DelegateSignature_Statics::NewProp_NewMaxHealth = { "NewMaxHealth", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(_Script_Aura_eventOnMaxHealthChangedSignature_Parms, NewMaxHealth), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UDelegateFunction_Aura_OnMaxHealthChangedSignature__DelegateSignature_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UDelegateFunction_Aura_OnMaxHealthChangedSignature__DelegateSignature_Statics::NewProp_NewMaxHealth,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UDelegateFunction_Aura_OnMaxHealthChangedSignature__DelegateSignature_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UDelegateFunction_Aura_OnMaxHealthChangedSignature__DelegateSignature_Statics::FuncParams = { (UObject*(*)())Z_Construct_UPackage__Script_Aura, nullptr, "OnMaxHealthChangedSignature__DelegateSignature", nullptr, nullptr, Z_Construct_UDelegateFunction_Aura_OnMaxHealthChangedSignature__DelegateSignature_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UDelegateFunction_Aura_OnMaxHealthChangedSignature__DelegateSignature_Statics::PropPointers), sizeof(Z_Construct_UDelegateFunction_Aura_OnMaxHealthChangedSignature__DelegateSignature_Statics::_Script_Aura_eventOnMaxHealthChangedSignature_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x00130000, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UDelegateFunction_Aura_OnMaxHealthChangedSignature__DelegateSignature_Statics::Function_MetaDataParams), Z_Construct_UDelegateFunction_Aura_OnMaxHealthChangedSignature__DelegateSignature_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UDelegateFunction_Aura_OnMaxHealthChangedSignature__DelegateSignature_Statics::_Script_Aura_eventOnMaxHealthChangedSignature_Parms) < MAX_uint16);
UFunction* Z_Construct_UDelegateFunction_Aura_OnMaxHealthChangedSignature__DelegateSignature()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UDelegateFunction_Aura_OnMaxHealthChangedSignature__DelegateSignature_Statics::FuncParams);
	}
	return ReturnFunction;
}
void FOnMaxHealthChangedSignature_DelegateWrapper(const FMulticastScriptDelegate& OnMaxHealthChangedSignature, float NewMaxHealth)
{
	struct _Script_Aura_eventOnMaxHealthChangedSignature_Parms
	{
		float NewMaxHealth;
	};
	_Script_Aura_eventOnMaxHealthChangedSignature_Parms Parms;
	Parms.NewMaxHealth=NewMaxHealth;
	OnMaxHealthChangedSignature.ProcessMulticastDelegate<UObject>(&Parms);
}
// End Delegate FOnMaxHealthChangedSignature

// Begin Delegate FOnManaChangedSignature
struct Z_Construct_UDelegateFunction_Aura_OnManaChangedSignature__DelegateSignature_Statics
{
	struct _Script_Aura_eventOnManaChangedSignature_Parms
	{
		float NewMana;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/UI/WidgetController/OverlayWidgetController.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FFloatPropertyParams NewProp_NewMana;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UDelegateFunction_Aura_OnManaChangedSignature__DelegateSignature_Statics::NewProp_NewMana = { "NewMana", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(_Script_Aura_eventOnManaChangedSignature_Parms, NewMana), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UDelegateFunction_Aura_OnManaChangedSignature__DelegateSignature_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UDelegateFunction_Aura_OnManaChangedSignature__DelegateSignature_Statics::NewProp_NewMana,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UDelegateFunction_Aura_OnManaChangedSignature__DelegateSignature_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UDelegateFunction_Aura_OnManaChangedSignature__DelegateSignature_Statics::FuncParams = { (UObject*(*)())Z_Construct_UPackage__Script_Aura, nullptr, "OnManaChangedSignature__DelegateSignature", nullptr, nullptr, Z_Construct_UDelegateFunction_Aura_OnManaChangedSignature__DelegateSignature_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UDelegateFunction_Aura_OnManaChangedSignature__DelegateSignature_Statics::PropPointers), sizeof(Z_Construct_UDelegateFunction_Aura_OnManaChangedSignature__DelegateSignature_Statics::_Script_Aura_eventOnManaChangedSignature_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x00130000, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UDelegateFunction_Aura_OnManaChangedSignature__DelegateSignature_Statics::Function_MetaDataParams), Z_Construct_UDelegateFunction_Aura_OnManaChangedSignature__DelegateSignature_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UDelegateFunction_Aura_OnManaChangedSignature__DelegateSignature_Statics::_Script_Aura_eventOnManaChangedSignature_Parms) < MAX_uint16);
UFunction* Z_Construct_UDelegateFunction_Aura_OnManaChangedSignature__DelegateSignature()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UDelegateFunction_Aura_OnManaChangedSignature__DelegateSignature_Statics::FuncParams);
	}
	return ReturnFunction;
}
void FOnManaChangedSignature_DelegateWrapper(const FMulticastScriptDelegate& OnManaChangedSignature, float NewMana)
{
	struct _Script_Aura_eventOnManaChangedSignature_Parms
	{
		float NewMana;
	};
	_Script_Aura_eventOnManaChangedSignature_Parms Parms;
	Parms.NewMana=NewMana;
	OnManaChangedSignature.ProcessMulticastDelegate<UObject>(&Parms);
}
// End Delegate FOnManaChangedSignature

// Begin Delegate FOnMaxManaChangedSignature
struct Z_Construct_UDelegateFunction_Aura_OnMaxManaChangedSignature__DelegateSignature_Statics
{
	struct _Script_Aura_eventOnMaxManaChangedSignature_Parms
	{
		float NewMaxMana;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/UI/WidgetController/OverlayWidgetController.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FFloatPropertyParams NewProp_NewMaxMana;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UDelegateFunction_Aura_OnMaxManaChangedSignature__DelegateSignature_Statics::NewProp_NewMaxMana = { "NewMaxMana", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(_Script_Aura_eventOnMaxManaChangedSignature_Parms, NewMaxMana), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UDelegateFunction_Aura_OnMaxManaChangedSignature__DelegateSignature_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UDelegateFunction_Aura_OnMaxManaChangedSignature__DelegateSignature_Statics::NewProp_NewMaxMana,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UDelegateFunction_Aura_OnMaxManaChangedSignature__DelegateSignature_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UDelegateFunction_Aura_OnMaxManaChangedSignature__DelegateSignature_Statics::FuncParams = { (UObject*(*)())Z_Construct_UPackage__Script_Aura, nullptr, "OnMaxManaChangedSignature__DelegateSignature", nullptr, nullptr, Z_Construct_UDelegateFunction_Aura_OnMaxManaChangedSignature__DelegateSignature_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UDelegateFunction_Aura_OnMaxManaChangedSignature__DelegateSignature_Statics::PropPointers), sizeof(Z_Construct_UDelegateFunction_Aura_OnMaxManaChangedSignature__DelegateSignature_Statics::_Script_Aura_eventOnMaxManaChangedSignature_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x00130000, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UDelegateFunction_Aura_OnMaxManaChangedSignature__DelegateSignature_Statics::Function_MetaDataParams), Z_Construct_UDelegateFunction_Aura_OnMaxManaChangedSignature__DelegateSignature_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UDelegateFunction_Aura_OnMaxManaChangedSignature__DelegateSignature_Statics::_Script_Aura_eventOnMaxManaChangedSignature_Parms) < MAX_uint16);
UFunction* Z_Construct_UDelegateFunction_Aura_OnMaxManaChangedSignature__DelegateSignature()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UDelegateFunction_Aura_OnMaxManaChangedSignature__DelegateSignature_Statics::FuncParams);
	}
	return ReturnFunction;
}
void FOnMaxManaChangedSignature_DelegateWrapper(const FMulticastScriptDelegate& OnMaxManaChangedSignature, float NewMaxMana)
{
	struct _Script_Aura_eventOnMaxManaChangedSignature_Parms
	{
		float NewMaxMana;
	};
	_Script_Aura_eventOnMaxManaChangedSignature_Parms Parms;
	Parms.NewMaxMana=NewMaxMana;
	OnMaxManaChangedSignature.ProcessMulticastDelegate<UObject>(&Parms);
}
// End Delegate FOnMaxManaChangedSignature

// Begin ScriptStruct FUIWidgetRow
static_assert(std::is_polymorphic<FUIWidgetRow>() == std::is_polymorphic<FTableRowBase>(), "USTRUCT FUIWidgetRow cannot be polymorphic unless super FTableRowBase is polymorphic");
static FStructRegistrationInfo Z_Registration_Info_UScriptStruct_UIWidgetRow;
class UScriptStruct* FUIWidgetRow::StaticStruct()
{
	if (!Z_Registration_Info_UScriptStruct_UIWidgetRow.OuterSingleton)
	{
		Z_Registration_Info_UScriptStruct_UIWidgetRow.OuterSingleton = GetStaticStruct(Z_Construct_UScriptStruct_FUIWidgetRow, (UObject*)Z_Construct_UPackage__Script_Aura(), TEXT("UIWidgetRow"));
	}
	return Z_Registration_Info_UScriptStruct_UIWidgetRow.OuterSingleton;
}
template<> AURA_API UScriptStruct* StaticStruct<FUIWidgetRow>()
{
	return FUIWidgetRow::StaticStruct();
}
struct Z_Construct_UScriptStruct_FUIWidgetRow_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "ModuleRelativePath", "Public/UI/WidgetController/OverlayWidgetController.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_MessageTag_MetaData[] = {
		{ "Category", "UIWidgetRow" },
		{ "ModuleRelativePath", "Public/UI/WidgetController/OverlayWidgetController.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Message_MetaData[] = {
		{ "Category", "UIWidgetRow" },
		{ "ModuleRelativePath", "Public/UI/WidgetController/OverlayWidgetController.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_MessageWidget_MetaData[] = {
		{ "Category", "UIWidgetRow" },
		{ "ModuleRelativePath", "Public/UI/WidgetController/OverlayWidgetController.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Image_MetaData[] = {
		{ "Category", "UIWidgetRow" },
		{ "ModuleRelativePath", "Public/UI/WidgetController/OverlayWidgetController.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStructPropertyParams NewProp_MessageTag;
	static const UECodeGen_Private::FTextPropertyParams NewProp_Message;
	static const UECodeGen_Private::FClassPropertyParams NewProp_MessageWidget;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_Image;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static void* NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FUIWidgetRow>();
	}
	static const UECodeGen_Private::FStructParams StructParams;
};
const UECodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FUIWidgetRow_Statics::NewProp_MessageTag = { "MessageTag", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FUIWidgetRow, MessageTag), Z_Construct_UScriptStruct_FGameplayTag, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_MessageTag_MetaData), NewProp_MessageTag_MetaData) }; // 1298103297
const UECodeGen_Private::FTextPropertyParams Z_Construct_UScriptStruct_FUIWidgetRow_Statics::NewProp_Message = { "Message", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Text, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FUIWidgetRow, Message), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Message_MetaData), NewProp_Message_MetaData) };
const UECodeGen_Private::FClassPropertyParams Z_Construct_UScriptStruct_FUIWidgetRow_Statics::NewProp_MessageWidget = { "MessageWidget", nullptr, (EPropertyFlags)0x0014000000000015, UECodeGen_Private::EPropertyGenFlags::Class, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FUIWidgetRow, MessageWidget), Z_Construct_UClass_UClass, Z_Construct_UClass_UAuraUserWidget_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_MessageWidget_MetaData), NewProp_MessageWidget_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UScriptStruct_FUIWidgetRow_Statics::NewProp_Image = { "Image", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FUIWidgetRow, Image), Z_Construct_UClass_UTexture2D_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Image_MetaData), NewProp_Image_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FUIWidgetRow_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FUIWidgetRow_Statics::NewProp_MessageTag,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FUIWidgetRow_Statics::NewProp_Message,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FUIWidgetRow_Statics::NewProp_MessageWidget,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FUIWidgetRow_Statics::NewProp_Image,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FUIWidgetRow_Statics::PropPointers) < 2048);
const UECodeGen_Private::FStructParams Z_Construct_UScriptStruct_FUIWidgetRow_Statics::StructParams = {
	(UObject* (*)())Z_Construct_UPackage__Script_Aura,
	Z_Construct_UScriptStruct_FTableRowBase,
	&NewStructOps,
	"UIWidgetRow",
	Z_Construct_UScriptStruct_FUIWidgetRow_Statics::PropPointers,
	UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FUIWidgetRow_Statics::PropPointers),
	sizeof(FUIWidgetRow),
	alignof(FUIWidgetRow),
	RF_Public|RF_Transient|RF_MarkAsNative,
	EStructFlags(0x00000001),
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FUIWidgetRow_Statics::Struct_MetaDataParams), Z_Construct_UScriptStruct_FUIWidgetRow_Statics::Struct_MetaDataParams)
};
UScriptStruct* Z_Construct_UScriptStruct_FUIWidgetRow()
{
	if (!Z_Registration_Info_UScriptStruct_UIWidgetRow.InnerSingleton)
	{
		UECodeGen_Private::ConstructUScriptStruct(Z_Registration_Info_UScriptStruct_UIWidgetRow.InnerSingleton, Z_Construct_UScriptStruct_FUIWidgetRow_Statics::StructParams);
	}
	return Z_Registration_Info_UScriptStruct_UIWidgetRow.InnerSingleton;
}
// End ScriptStruct FUIWidgetRow

// Begin Class UOverlayWidgetController
void UOverlayWidgetController::StaticRegisterNativesUOverlayWidgetController()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UOverlayWidgetController);
UClass* Z_Construct_UClass_UOverlayWidgetController_NoRegister()
{
	return UOverlayWidgetController::StaticClass();
}
struct Z_Construct_UClass_UOverlayWidgetController_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "IncludePath", "UI/WidgetController/OverlayWidgetController.h" },
		{ "IsBlueprintBase", "true" },
		{ "ModuleRelativePath", "Public/UI/WidgetController/OverlayWidgetController.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OnHealthChanged_MetaData[] = {
		{ "Category", "GAS|Attributes" },
		{ "ModuleRelativePath", "Public/UI/WidgetController/OverlayWidgetController.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OnMaxHealthChanged_MetaData[] = {
		{ "Category", "GAS|Attributes" },
		{ "ModuleRelativePath", "Public/UI/WidgetController/OverlayWidgetController.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OnManaChanged_MetaData[] = {
		{ "Category", "GAS|Attributes" },
		{ "ModuleRelativePath", "Public/UI/WidgetController/OverlayWidgetController.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OnMaxManaChanged_MetaData[] = {
		{ "Category", "GAS|Attributes" },
		{ "ModuleRelativePath", "Public/UI/WidgetController/OverlayWidgetController.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_MessageWidgetDataTable_MetaData[] = {
		{ "Category", "Widget Data" },
		{ "ModuleRelativePath", "Public/UI/WidgetController/OverlayWidgetController.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FMulticastDelegatePropertyParams NewProp_OnHealthChanged;
	static const UECodeGen_Private::FMulticastDelegatePropertyParams NewProp_OnMaxHealthChanged;
	static const UECodeGen_Private::FMulticastDelegatePropertyParams NewProp_OnManaChanged;
	static const UECodeGen_Private::FMulticastDelegatePropertyParams NewProp_OnMaxManaChanged;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_MessageWidgetDataTable;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UOverlayWidgetController>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FMulticastDelegatePropertyParams Z_Construct_UClass_UOverlayWidgetController_Statics::NewProp_OnHealthChanged = { "OnHealthChanged", nullptr, (EPropertyFlags)0x0010000010080000, UECodeGen_Private::EPropertyGenFlags::InlineMulticastDelegate, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UOverlayWidgetController, OnHealthChanged), Z_Construct_UDelegateFunction_Aura_OnHealthChangedSignature__DelegateSignature, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_OnHealthChanged_MetaData), NewProp_OnHealthChanged_MetaData) }; // 2161055909
const UECodeGen_Private::FMulticastDelegatePropertyParams Z_Construct_UClass_UOverlayWidgetController_Statics::NewProp_OnMaxHealthChanged = { "OnMaxHealthChanged", nullptr, (EPropertyFlags)0x0010000010080000, UECodeGen_Private::EPropertyGenFlags::InlineMulticastDelegate, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UOverlayWidgetController, OnMaxHealthChanged), Z_Construct_UDelegateFunction_Aura_OnMaxHealthChangedSignature__DelegateSignature, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_OnMaxHealthChanged_MetaData), NewProp_OnMaxHealthChanged_MetaData) }; // 2209117285
const UECodeGen_Private::FMulticastDelegatePropertyParams Z_Construct_UClass_UOverlayWidgetController_Statics::NewProp_OnManaChanged = { "OnManaChanged", nullptr, (EPropertyFlags)0x0010000010080000, UECodeGen_Private::EPropertyGenFlags::InlineMulticastDelegate, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UOverlayWidgetController, OnManaChanged), Z_Construct_UDelegateFunction_Aura_OnManaChangedSignature__DelegateSignature, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_OnManaChanged_MetaData), NewProp_OnManaChanged_MetaData) }; // 4100557847
const UECodeGen_Private::FMulticastDelegatePropertyParams Z_Construct_UClass_UOverlayWidgetController_Statics::NewProp_OnMaxManaChanged = { "OnMaxManaChanged", nullptr, (EPropertyFlags)0x0010000010080000, UECodeGen_Private::EPropertyGenFlags::InlineMulticastDelegate, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UOverlayWidgetController, OnMaxManaChanged), Z_Construct_UDelegateFunction_Aura_OnMaxManaChangedSignature__DelegateSignature, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_OnMaxManaChanged_MetaData), NewProp_OnMaxManaChanged_MetaData) }; // 354488370
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_UOverlayWidgetController_Statics::NewProp_MessageWidgetDataTable = { "MessageWidgetDataTable", nullptr, (EPropertyFlags)0x0124080000010015, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UOverlayWidgetController, MessageWidgetDataTable), Z_Construct_UClass_UDataTable_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_MessageWidgetDataTable_MetaData), NewProp_MessageWidgetDataTable_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UOverlayWidgetController_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UOverlayWidgetController_Statics::NewProp_OnHealthChanged,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UOverlayWidgetController_Statics::NewProp_OnMaxHealthChanged,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UOverlayWidgetController_Statics::NewProp_OnManaChanged,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UOverlayWidgetController_Statics::NewProp_OnMaxManaChanged,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UOverlayWidgetController_Statics::NewProp_MessageWidgetDataTable,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UOverlayWidgetController_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_UOverlayWidgetController_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UAuraWidgetController,
	(UObject* (*)())Z_Construct_UPackage__Script_Aura,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UOverlayWidgetController_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UOverlayWidgetController_Statics::ClassParams = {
	&UOverlayWidgetController::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_UOverlayWidgetController_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_UOverlayWidgetController_Statics::PropPointers),
	0,
	0x009000A0u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UOverlayWidgetController_Statics::Class_MetaDataParams), Z_Construct_UClass_UOverlayWidgetController_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UOverlayWidgetController()
{
	if (!Z_Registration_Info_UClass_UOverlayWidgetController.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UOverlayWidgetController.OuterSingleton, Z_Construct_UClass_UOverlayWidgetController_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UOverlayWidgetController.OuterSingleton;
}
template<> AURA_API UClass* StaticClass<UOverlayWidgetController>()
{
	return UOverlayWidgetController::StaticClass();
}
UOverlayWidgetController::UOverlayWidgetController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UOverlayWidgetController);
UOverlayWidgetController::~UOverlayWidgetController() {}
// End Class UOverlayWidgetController

// Begin Registration
struct Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_UI_WidgetController_OverlayWidgetController_h_Statics
{
	static constexpr FStructRegisterCompiledInInfo ScriptStructInfo[] = {
		{ FUIWidgetRow::StaticStruct, Z_Construct_UScriptStruct_FUIWidgetRow_Statics::NewStructOps, TEXT("UIWidgetRow"), &Z_Registration_Info_UScriptStruct_UIWidgetRow, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FUIWidgetRow), 2824825175U) },
	};
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UOverlayWidgetController, UOverlayWidgetController::StaticClass, TEXT("UOverlayWidgetController"), &Z_Registration_Info_UClass_UOverlayWidgetController, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UOverlayWidgetController), 1498140267U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_UI_WidgetController_OverlayWidgetController_h_181359810(TEXT("/Script/Aura"),
	Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_UI_WidgetController_OverlayWidgetController_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_UI_WidgetController_OverlayWidgetController_h_Statics::ClassInfo),
	Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_UI_WidgetController_OverlayWidgetController_h_Statics::ScriptStructInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_ALPHA_UnrealProject_Course_Gameplay_Ability_System___Top_Down_RPG_Aura_Source_Aura_Public_UI_WidgetController_OverlayWidgetController_h_Statics::ScriptStructInfo),
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
