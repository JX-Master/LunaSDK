/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file TypeInfo.hpp
* @author JXMaster
* @date 2022/5/14
* @brief The type system for Luna.
*/
#pragma once
#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif
#include "Span.hpp"
namespace Luna
{
	//! @addtogroup Runtime
	//! @{
	//! @defgroup RuntimeType Type reflection
	//! @}

	//! @addtogroup RuntimeType
	//! @{
	
	//! The opaque pointer that points to one type object.
	using typeinfo_t = opaque_t;

	//! Gets the type object from one type GUID.
	//! @param[in] guid The GUID of the type.
	//! @return Returns the type object for the GUID. Returns `nullptr` if the specified type is not found.
	LUNA_RUNTIME_API typeinfo_t get_type_by_guid(const Guid& guid);

	//! The functional obejct that can be overloaded to define custom behavior of @ref typeof for user-defined types.
	template <typename _Ty> struct typeof_t
	{
		typeinfo_t operator()() const { return get_type_by_guid(_Ty::__guid); }
	};

	//! Gets the type object of the specified type.
	//! @return Returns the type object of the specified type. Returns `nullptr` if the type is not registered.
	//! @remark This function calls @ref typeof_t internally to get the type object of the specified type.
	template <typename _Ty>
	inline typeinfo_t typeof() { return typeof_t<_Ty>()(); }

	//! Gets the type object of `void` type.
	//! @return Returns the type object of `void` type.
	LUNA_RUNTIME_API typeinfo_t void_type();
	//! Gets the type object of `u8` type.
	//! @return Returns the type object of `u8` type.
	LUNA_RUNTIME_API typeinfo_t u8_type();
	//! Gets the type object of `i8` type.
	//! @return Returns the type object of `i8` type.
	LUNA_RUNTIME_API typeinfo_t i8_type();
	//! Gets the type object of `u16` type.
	//! @return Returns the type object of `u16` type.
	LUNA_RUNTIME_API typeinfo_t u16_type();
	//! Gets the type object of `i16` type.
	//! @return Returns the type object of `i16` type.
	LUNA_RUNTIME_API typeinfo_t i16_type();
	//! Gets the type object of `u32` type.
	//! @return Returns the type object of `u32` type.
	LUNA_RUNTIME_API typeinfo_t u32_type();
	//! Gets the type object of `i32` type.
	//! @return Returns the type object of `i32` type.
	LUNA_RUNTIME_API typeinfo_t i32_type();
	//! Gets the type object of `u64` type.
	//! @return Returns the type object of `u64` type.
	LUNA_RUNTIME_API typeinfo_t u64_type();
	//! Gets the type object of `i64` type.
	//! @return Returns the type object of `i64` type.
	LUNA_RUNTIME_API typeinfo_t i64_type();
	//! Gets the type object of `usize` type.
	//! @return Returns the type object of `usize` type.
	LUNA_RUNTIME_API typeinfo_t usize_type();
	//! Gets the type object of `isize` type.
	//! @return Returns the type object of `isize` type.
	LUNA_RUNTIME_API typeinfo_t isize_type();
	//! Gets the type object of `f32` type.
	//! @return Returns the type object of `f32` type.
	LUNA_RUNTIME_API typeinfo_t f32_type();
	//! Gets the type object of `f64` type.
	//! @return Returns the type object of `f64` type.
	LUNA_RUNTIME_API typeinfo_t f64_type();
	//! Gets the type object of `c8` type.
	//! @return Returns the type object of `c8` type.
	LUNA_RUNTIME_API typeinfo_t c8_type();
	//! Gets the type object of `c16` type.
	//! @return Returns the type object of `c16` type.
	LUNA_RUNTIME_API typeinfo_t c16_type();
	//! Gets the type object of `c32` type.
	//! @return Returns the type object of `c32` type.
	LUNA_RUNTIME_API typeinfo_t c32_type();
	//! Gets the type object of `bool` type.
	//! @return Returns the type object of `bool` type.
	LUNA_RUNTIME_API typeinfo_t boolean_type();

	template <> struct typeof_t<u8> { typeinfo_t operator()() const { return u8_type(); } };
	template <> struct typeof_t<i8> { typeinfo_t operator()() const { return i8_type(); } };
	template <> struct typeof_t<u16> { typeinfo_t operator()() const { return u16_type(); } };
	template <> struct typeof_t<i16> { typeinfo_t operator()() const { return i16_type(); } };
	template <> struct typeof_t<u32> { typeinfo_t operator()() const { return u32_type(); } };
	template <> struct typeof_t<i32> { typeinfo_t operator()() const { return i32_type(); } };
	template <> struct typeof_t<u64> { typeinfo_t operator()() const { return u64_type(); } };
	template <> struct typeof_t<i64> { typeinfo_t operator()() const { return i64_type(); } };
	template <> struct typeof_t<f32> { typeinfo_t operator()() const { return f32_type(); } };
	template <> struct typeof_t<f64> { typeinfo_t operator()() const { return f64_type(); } };
	template <> struct typeof_t<c8> { typeinfo_t operator()() const { return c8_type(); } };
	template <> struct typeof_t<c16> { typeinfo_t operator()() const { return c16_type(); } };
	template <> struct typeof_t<c32> { typeinfo_t operator()() const { return c32_type(); } };
	template <> struct typeof_t<bool> { typeinfo_t operator()() const { return boolean_type(); } };

	//! Gets the type object of `Guid` type.
	//! @return Returns the type object of `Guid` type.
	LUNA_RUNTIME_API typeinfo_t guid_type();
	//! Gets the type object of `Version` type.
	//! @return Returns the type object of `Version` type.
	LUNA_RUNTIME_API typeinfo_t version_type();

	template <> struct typeof_t<Guid> { typeinfo_t operator()() const { return guid_type(); } };
	template <> struct typeof_t<Version> { typeinfo_t operator()() const { return version_type(); } };
	
	//! Gets the type object of `Pair` generic type.
	//! @return Returns the type object of `Pair` generic type.
	LUNA_RUNTIME_API typeinfo_t pair_type();
	template <typename _Ty1, typename _Ty2> struct typeof_t<Pair<_Ty1, _Ty2>> 
	{ typeinfo_t operator()() const { return get_generic_instanced_type(pair_type(), {typeof<_Ty1>(), typeof<_Ty2>()}); } };
	
	//! Gets one instanced type of one generic type.
	//! @param[in] generic_type The generic type.
	//! @param[in] generic_arguments The type arguments that are used to query the instanced type.
	//! @return Returns the instanced type requested.
	LUNA_RUNTIME_API typeinfo_t get_generic_instanced_type(typeinfo_t generic_type, Span<const typeinfo_t> generic_arguments);

	template <typename _Ty>
	struct EnumTypeInfo {};

	//! @}
}

//! @addtogroup RuntimeType
//! @{

//! Declares the name and guid for one structure or class type.
//! @param[in] _name The name string of the type.
//! @param[in] _guid The GUID string of the type.
//! @remark Add this macro to the type body. For exmaple:
//! ```
//! struct MyType
//! {
//! 	lustruct("MyType", "{dbeecd7a-2dc5-423e-8e20-7521826c3f06}");
//! 	// other members...
//! };
//! ```
#define lustruct(_name, _guid) static constexpr const Luna::c8* __name = _name; static constexpr Luna::Guid __guid { Luna::Guid(u8##_guid) };

//! Declares one property used in @ref Luna::register_struct_type.
//! @param[in] _struct The outer structure or class name.
//! @param[in] _type The property type.
//! @param[in] _name The property name.
#define luproperty(_struct, _type, _name) {#_name, typeof<_type>(), offsetof(_struct, _name)}

//! Declares the name and guid for one enumeration type.
//! @details This macro must be defined directly in `Luna` namespace.
//! @param[in] _type The enumeration type.
//! @param[in] _name The name string of the type.
//! @param[in] _guid The GUID string of the type.
#define luenum(_type, _name, _guid) template <> struct EnumTypeInfo<_type> { static constexpr const Luna::c8* __name = _name; static constexpr Luna::Guid __guid { Luna::Guid(u8##_guid) }; }; template <> struct typeof_t<_type> { typeinfo_t operator()() const { return get_type_by_guid(EnumTypeInfo<_type>::__guid); } };

//! Declares one option used in @ref Luna::register_enum_type.
//! @param[in] _enum The outer enumeration type.
//! @param[in] _item The option name.
#define luoption(_enum, _item) {#_item, (i64)(_enum::_item)}

//! @}