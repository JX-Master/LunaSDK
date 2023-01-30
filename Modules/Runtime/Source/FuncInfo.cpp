/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FuncInfo.cpp
* @author JXMaster
* @date 2022/6/7
*/
#pragma once
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "FuncInfo.hpp"
#include "../UniquePtr.hpp"
#include "../UnorderedMultiMap.hpp"
#include "../HashMap.hpp"
#include "OS.hpp"

namespace Luna
{
	Vector<UniquePtr<FuncInfo>> g_func_registry;
	handle_t g_func_registry_lock;

	UnorderedMultiMap<Name, FuncInfo*> g_func_name_map;
	HashMap<Guid, FuncInfo*> g_func_guid_map;

	void function_registry_init()
	{
		g_func_registry_lock = OS::new_mutex();
	}
	void function_registry_close()
	{
		g_func_registry.clear();
		g_func_registry.shrink_to_fit();
		g_func_name_map.clear();
		g_func_guid_map.clear();
		g_func_guid_map.shrink_to_fit();
		OS::delete_mutex(g_func_registry_lock);
	}
	LUNA_RUNTIME_API funcinfo_t register_function(native_func_t* func, const FunctionDesc& desc)
	{
		OSMutexGuard guard(g_func_registry_lock);
		funcinfo_t function = get_function_by_guid(desc.guid);
		if (function) return function;
		function = get_function_by_name(desc.name, desc.alias);
		if (function) return function;
		UniquePtr<FuncInfo> f(memnew<FuncInfo>());
		f->kind = FuncInfoKind::global;
		f->throws = false;
		f->guid = desc.guid;
		f->name = desc.name;
		f->alias = desc.alias;
		f->native_function_pointer = func;
		f->parameters = desc.parameters;
		f->context_type = desc.context_type;
		f->return_value_type = desc.return_value_type;
		f->return_value_kind = desc.return_value_kind;
		funcinfo_t r = f.get();
		g_func_name_map.insert(make_pair(f->name, f.get()));
		g_func_guid_map.insert(make_pair(f->guid, f.get()));
		g_func_registry.push_back(move(f));
		return r;
	}
	LUNA_RUNTIME_API funcinfo_t register_throwable_function(native_func_throws_t* func, const FunctionDesc& desc)
	{
		OSMutexGuard guard(g_func_registry_lock);
		funcinfo_t function = get_function_by_guid(desc.guid);
		if (function) return function;
		function = get_function_by_name(desc.name, desc.alias);
		if (function) return function;
		UniquePtr<FuncInfo> f(memnew<FuncInfo>());
		f->kind = FuncInfoKind::global;
		f->throws = true;
		f->guid = desc.guid;
		f->name = desc.name;
		f->alias = desc.alias;
		f->native_function_pointer = func;
		f->parameters = desc.parameters;
		f->context_type = desc.context_type;
		f->return_value_type = desc.return_value_type;
		f->return_value_kind = desc.return_value_kind;
		funcinfo_t r = f.get();
		g_func_name_map.insert(make_pair(f->name, f.get()));
		g_func_guid_map.insert(make_pair(f->guid, f.get()));
		g_func_registry.push_back(move(f));
		return r;
	}
	LUNA_RUNTIME_API funcinfo_t get_function_by_name(const Name& name, const Name& alias)
	{
		OSMutexGuard guard(g_func_registry_lock);
		auto range = g_func_name_map.equal_range(name);
		if (range.first == range.second) return nullptr;
		for (auto iter = range.first; iter != range.second; ++iter)
		{
			FuncInfo* f = iter->second;
			if (f->alias != alias) continue;
			return (funcinfo_t)f;
		}
		return nullptr;
	}
	LUNA_RUNTIME_API funcinfo_t get_function_by_guid(const Guid& guid)
	{
		OSMutexGuard guard(g_func_registry_lock);
		auto iter = g_func_guid_map.find(guid);
		if (iter == g_func_guid_map.end()) return nullptr;
		return (funcinfo_t)(iter->second);
	}
	LUNA_RUNTIME_API void call(funcinfo_t func, void* ret, void* params)
	{
		FuncInfo* f = (FuncInfo*)func;
		lucheck(!f->throws);
		((native_func_t*)(f->native_function_pointer))(func, ret, params);
	}
	LUNA_RUNTIME_API RV call_throws(funcinfo_t func, void* ret, void* params)
	{
		FuncInfo* f = (FuncInfo*)func;
		lucheck(f->throws);
		return ((native_func_throws_t*)(f->native_function_pointer))(func, ret, params);
	}
}