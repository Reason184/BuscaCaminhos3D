// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef MYPROJECT_TextWidget_generated_h
#error "TextWidget.generated.h already included, missing '#pragma once' in TextWidget.h"
#endif
#define MYPROJECT_TextWidget_generated_h

#define MyProject_Source_MyProject_Public_TextWidget_h_15_SPARSE_DATA
#define MyProject_Source_MyProject_Public_TextWidget_h_15_RPC_WRAPPERS
#define MyProject_Source_MyProject_Public_TextWidget_h_15_RPC_WRAPPERS_NO_PURE_DECLS
#define MyProject_Source_MyProject_Public_TextWidget_h_15_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUTextWidget(); \
	friend struct Z_Construct_UClass_UTextWidget_Statics; \
public: \
	DECLARE_CLASS(UTextWidget, UUserWidget, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/MyProject"), NO_API) \
	DECLARE_SERIALIZER(UTextWidget)


#define MyProject_Source_MyProject_Public_TextWidget_h_15_INCLASS \
private: \
	static void StaticRegisterNativesUTextWidget(); \
	friend struct Z_Construct_UClass_UTextWidget_Statics; \
public: \
	DECLARE_CLASS(UTextWidget, UUserWidget, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/MyProject"), NO_API) \
	DECLARE_SERIALIZER(UTextWidget)


#define MyProject_Source_MyProject_Public_TextWidget_h_15_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UTextWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UTextWidget) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UTextWidget); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UTextWidget); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UTextWidget(UTextWidget&&); \
	NO_API UTextWidget(const UTextWidget&); \
public:


#define MyProject_Source_MyProject_Public_TextWidget_h_15_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UTextWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer) { }; \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UTextWidget(UTextWidget&&); \
	NO_API UTextWidget(const UTextWidget&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UTextWidget); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UTextWidget); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UTextWidget)


#define MyProject_Source_MyProject_Public_TextWidget_h_15_PRIVATE_PROPERTY_OFFSET \
	FORCEINLINE static uint32 __PPO__TextLabel() { return STRUCT_OFFSET(UTextWidget, TextLabel); }


#define MyProject_Source_MyProject_Public_TextWidget_h_12_PROLOG
#define MyProject_Source_MyProject_Public_TextWidget_h_15_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	MyProject_Source_MyProject_Public_TextWidget_h_15_PRIVATE_PROPERTY_OFFSET \
	MyProject_Source_MyProject_Public_TextWidget_h_15_SPARSE_DATA \
	MyProject_Source_MyProject_Public_TextWidget_h_15_RPC_WRAPPERS \
	MyProject_Source_MyProject_Public_TextWidget_h_15_INCLASS \
	MyProject_Source_MyProject_Public_TextWidget_h_15_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define MyProject_Source_MyProject_Public_TextWidget_h_15_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	MyProject_Source_MyProject_Public_TextWidget_h_15_PRIVATE_PROPERTY_OFFSET \
	MyProject_Source_MyProject_Public_TextWidget_h_15_SPARSE_DATA \
	MyProject_Source_MyProject_Public_TextWidget_h_15_RPC_WRAPPERS_NO_PURE_DECLS \
	MyProject_Source_MyProject_Public_TextWidget_h_15_INCLASS_NO_PURE_DECLS \
	MyProject_Source_MyProject_Public_TextWidget_h_15_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> MYPROJECT_API UClass* StaticClass<class UTextWidget>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID MyProject_Source_MyProject_Public_TextWidget_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
