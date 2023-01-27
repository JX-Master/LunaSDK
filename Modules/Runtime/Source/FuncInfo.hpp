/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FuncInfo.hpp
* @author JXMaster
* @date 2022/6/7
*/
#pragma once
#include "../FuncInfo.hpp"

namespace Luna
{
	enum class FuncInfoKind
	{
		global = 0,
		contextual = 1,
	};
	struct FuncInfo
	{
		FuncInfoKind kind;
		bool throws;
		Guid guid;
		Name name;
		Name alias;
		void* native_function_pointer;
		Vector<FunctionParameter> parameters;
		typeinfo_t context_type;
		typeinfo_t return_value_type;
		FunctionParameterKind return_value_kind;
	};

	void function_registry_init();
	void function_registry_close();
}