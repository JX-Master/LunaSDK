/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FuncInfo.hpp
* @author JXMaster
* @date 2022/6/5
*/
#pragma once
#include "TypeInfo.hpp"
#include "Result.hpp"

namespace Luna
{
	using funcinfo_t = opaque_t;

	using native_func_t = ErrCode(funcinfo_t info, void* ctx, void* outputs, void* inputs);

	enum class FunctionParameterKind : u8
	{
		value = 0,
		reference = 1,
		mutable_reference = 2,
		rvalue_reference = 3,
		mutable_rvalue_reference = 4,
	};

	struct FunctionParameter
	{
		Name name;
		typeinfo_t type;
		FunctionParameterKind kind;

		bool operator==(const FunctionParameter& rhs) const
		{
			return type == rhs.type && kind == rhs.kind;
		}
		bool operator!=(const FunctionParameter& rhs) const
		{
			return !(*this == rhs);
		}
	};

	enum class FunctionType : u8
	{
		global = 0,
		method = 1,
		getter = 2,
		setter = 3,
	};

	struct FunctionSignature
	{
		Vector<FunctionParameter> inputs;
		Vector<FunctionParameter> outputs;
		bool throws;

		FunctionSignature(const Vector<FunctionParameter>& inputs, const Vector<FunctionParameter>& outputs, bool throws = false) :
			inputs(inputs),
			outputs(outputs),
			throws(throws) {}
	};

	LUNA_RUNTIME_API funcinfo_t register_function(native_func_t* func, const Name& name, const Name& alias, const Guid& guid, const FunctionSignature& signature);
	LUNA_RUNTIME_API funcinfo_t register_method(native_func_t* func, typeinfo_t outer_type, const Name& name, const Name& alias, const Guid& guid, const FunctionSignature& signature);
	LUNA_RUNTIME_API funcinfo_t register_getter(native_func_t* func, typeinfo_t outer_type, const Name& name, const Name& alias, const Guid& guid, const FunctionSignature& signature);
	LUNA_RUNTIME_API funcinfo_t register_setter(native_func_t* func, typeinfo_t outer_type, const Name& name, const Name& alias, const Guid& guid, const FunctionSignature& signature);

	LUNA_RUNTIME_API funcinfo_t get_function_by_name(const Name& name, const Name& alias = Name());
	LUNA_RUNTIME_API funcinfo_t get_function_by_guid(const Guid& guid);

	LUNA_RUNTIME_API ErrCode call(funcinfo_t func, void* ret, void* params);

}