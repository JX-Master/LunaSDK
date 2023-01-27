/*
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
#include "Base.hpp"
namespace Luna
{
	using typeinfo_t = opaque_t;

	LUNA_RUNTIME_API typeinfo_t get_type_by_guid(const Guid& guid);

	//! Overload this to implement `typeof`
	template <typename _Ty> struct typeof_t
	{
		typeinfo_t operator()() const { return get_type_by_guid(_Ty::__guid); }
	};

	template <typename _Ty>
	inline typeinfo_t typeof() { return typeof_t<_Ty>()(); }

	LUNA_RUNTIME_API typeinfo_t void_type();
	LUNA_RUNTIME_API typeinfo_t u8_type();
	LUNA_RUNTIME_API typeinfo_t i8_type();
	LUNA_RUNTIME_API typeinfo_t u16_type();
	LUNA_RUNTIME_API typeinfo_t i16_type();
	LUNA_RUNTIME_API typeinfo_t u32_type();
	LUNA_RUNTIME_API typeinfo_t i32_type();
	LUNA_RUNTIME_API typeinfo_t u64_type();
	LUNA_RUNTIME_API typeinfo_t i64_type();
	LUNA_RUNTIME_API typeinfo_t usize_type();
	LUNA_RUNTIME_API typeinfo_t isize_type();
	LUNA_RUNTIME_API typeinfo_t f32_type();
	LUNA_RUNTIME_API typeinfo_t f64_type();
	LUNA_RUNTIME_API typeinfo_t c8_type();
	LUNA_RUNTIME_API typeinfo_t c16_type();
	LUNA_RUNTIME_API typeinfo_t c32_type();
	LUNA_RUNTIME_API typeinfo_t boolean_type();
	LUNA_RUNTIME_API typeinfo_t pointer_type();

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

	LUNA_RUNTIME_API typeinfo_t guid_type();
	LUNA_RUNTIME_API typeinfo_t version_type();

	template <> struct typeof_t<Guid> { typeinfo_t operator()() const { return guid_type(); } };
	template <> struct typeof_t<Version> { typeinfo_t operator()() const { return version_type(); } };
	
	LUNA_RUNTIME_API typeinfo_t pair_type();
	template <typename _Ty1, typename _Ty2> struct typeof_t<Pair<_Ty1, _Ty2>> 
	{ typeinfo_t operator()() const { return get_generic_instanced_type(pair_type(), {typeof<_Ty1>(), typeof<_Ty2>()}); } };

	LUNA_RUNTIME_API typeinfo_t get_generic_instanced_type(typeinfo_t generic_type, const typeinfo_t* generic_arguments, usize num_generic_arguments);
	inline typeinfo_t get_generic_instanced_type(typeinfo_t generic_type, InitializerList<typeinfo_t> generic_arguments)
	{
		return get_generic_instanced_type(generic_type, generic_arguments.begin(), generic_arguments.size());
	}

	template <typename _Ty>
	struct EnumTypeInfo {};
}

#define lustruct(_name, _guid) static constexpr const Luna::c8* __name = _name; static constexpr Luna::Guid __guid { Luna::Guid(u8##_guid) };
#define luproperty(_struct, _type, _name) {#_name, typeof<_type>(), offsetof(_struct, _name)}
#define luenum(_type, _name, _guid) template <> struct EnumTypeInfo<_type> { static constexpr const Luna::c8* __name = _name; static constexpr Luna::Guid __guid { Luna::Guid(u8##_guid) }; }; template <> struct typeof_t<_type> { typeinfo_t operator()() const { return get_type_by_guid(EnumTypeInfo<_type>::__guid); } };
#define luoption(_enum, _item) {#_item, (i64)(_enum::_item)}