/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Error.cpp
* @author JXMaster
* @date 2020/11/24
*/
#include <Runtime/PlatformDefines.hpp>

#define LUNA_RUNTIME_API LUNA_EXPORT
#include <Runtime/Error.hpp>
#include <Runtime/HashMap.hpp>
#include "OS.hpp"
#include "../SpinLock.hpp"
namespace Luna
{
	SpinLock g_error_mtx;
	opaque_t g_error_tls;

	struct ErrCodeRegistry
	{
		c8* name;
		errcat_t belonging_error_category;

		ErrCodeRegistry(const c8* name) :
			belonging_error_category(0)
		{
			usize l = strlen(name);
			c8* str = (c8*)memalloc((l + 1) * sizeof(c8));
			memcpy(str, name, (l + 1) * sizeof(c8));
			this->name = str;
		}
		ErrCodeRegistry(const ErrCodeRegistry&) = delete;
		ErrCodeRegistry(ErrCodeRegistry&& rhs) :
			name(rhs.name),
			belonging_error_category(rhs.belonging_error_category)
		{
			rhs.name = nullptr;
		}
		~ErrCodeRegistry()
		{
			if (name)
			{
				memdelete(name);
				name = nullptr;
			}
		}
	};

	struct ErrCategroyRegistry
	{
		c8* name;
		errcat_t belonging_error_category;
		Vector<ErrCode> codes;
		Vector<errcat_t> subcategories;

		ErrCategroyRegistry(const c8* name, usize name_sz) :
			belonging_error_category(0)
		{
			c8* str = (c8*)memalloc((name_sz + 1) * sizeof(c8));
			memcpy(str, name, name_sz * sizeof(c8));
			str[name_sz] = '\0';
			this->name = str;
		}
		ErrCategroyRegistry(const ErrCategroyRegistry&) = delete;
		ErrCategroyRegistry(ErrCategroyRegistry&& rhs) :
			name(rhs.name),
			belonging_error_category(rhs.belonging_error_category),
			codes(move(rhs.codes)),
			subcategories(move(rhs.subcategories))
		{
			rhs.name = nullptr;
		}
		~ErrCategroyRegistry()
		{
			if (name)
			{
				memdelete(name);
				name = nullptr;
			}
		}
	};

	Unconstructed<HashMap<ErrCode, ErrCodeRegistry>> g_errcode_registry;
	Unconstructed<HashMap<errcat_t, ErrCategroyRegistry>> g_errcat_registry;

	void error_destructor(void* cookie)
	{
		Error* err = (Error*)cookie;
		memdelete(err);
	}

	void error_init()
	{
		g_errcat_registry.construct();
		g_errcode_registry.construct();
		g_error_tls = OS::tls_alloc(error_destructor);
	}

	void error_close()
	{
		Error* err = (Error*)OS::tls_get(g_error_tls);
		if (err)
		{
			memdelete(err);
			OS::tls_set(g_error_tls, nullptr);
		}
		OS::tls_free(g_error_tls);
		g_errcode_registry.destruct();
		g_errcat_registry.destruct();
	}

	static errcat_t interal_get_error_category_by_name(const c8* errcat_name, ErrCategroyRegistry** out_errtype_registry)
	{
		lucheck(errcat_name && *errcat_name != '\0');
		usize errcat_name_sz = strlen(errcat_name);
		usize h = memhash<usize>(errcat_name, errcat_name_sz * sizeof(c8));
		auto& r = g_errcat_registry.get();
		auto iter = r.find(h);
		if (iter == r.end())
		{
			auto it2 = r.insert(make_pair(h, ErrCategroyRegistry(errcat_name, errcat_name_sz)));
			if (out_errtype_registry)
			{
				*out_errtype_registry = &(it2.first->second);
			}
		}
		else
		{
			if (out_errtype_registry)
			{
				*out_errtype_registry = &(iter->second);
			}
		}
		return h;
	}

	LUNA_RUNTIME_API ErrCode get_error_code_by_name(const c8* errcat_name, const c8* errcode_name)
	{
		lucheck(errcode_name && errcat_name && *errcode_name != '\0' && *errcat_name != '\0');
		usize h = strhash<u64>(errcat_name);
		h = strhash<u64>(errcode_name, h);
		auto& r = g_errcode_registry.get();
		LockGuard g(g_error_mtx);
		auto iter = r.find(ErrCode(h));
		if (iter == r.end())
		{
			auto it2 = r.insert(make_pair(h, ErrCodeRegistry(errcode_name)));
			ErrCategroyRegistry* errcat;
			it2.first->second.belonging_error_category = interal_get_error_category_by_name(errcat_name, &errcat);
			errcat->codes.push_back(ErrCode(h));
		}
		return ErrCode(h);
	}

	LUNA_RUNTIME_API errcat_t get_error_category_by_name(const c8* errcat_name)
	{
		LockGuard g(g_error_mtx);
		errcat_t r = interal_get_error_category_by_name(errcat_name, nullptr);
		return r;
	}

	LUNA_RUNTIME_API void set_error_subcategory(errcat_t parent_category, errcat_t child_category)
	{
		LockGuard g(g_error_mtx);
		auto& r = g_errcat_registry.get();
		auto iter1 = r.find(parent_category);
		auto iter2 = r.find(child_category);
		if (iter1 != r.end() && iter2 != r.end())
		{
			// prevent being set multiple times.
			if (iter2->second.belonging_error_category == 0)
			{
				iter2->second.belonging_error_category = parent_category;
				iter1->second.subcategories.push_back(child_category);
			}
		}
	}

	LUNA_RUNTIME_API const c8* get_error_code_name(ErrCode err_code)
	{
		LockGuard g(g_error_mtx);
		auto iter = g_errcode_registry.get().find(err_code);
		if (iter == g_errcode_registry.get().end())
		{
			return "";
		}
		return iter->second.name;
	}

	LUNA_RUNTIME_API const c8* get_error_category_name(errcat_t err_category)
	{
		LockGuard g(g_error_mtx);
		auto iter = g_errcat_registry.get().find(err_category);
		if (iter == g_errcat_registry.get().end())
		{
			return "";
		}
		return iter->second.name;
	}

	LUNA_RUNTIME_API errcat_t get_error_code_category(ErrCode err_code)
	{
		LockGuard g(g_error_mtx);
		auto iter = g_errcode_registry.get().find(err_code);
		if (iter == g_errcode_registry.get().end())
		{
			return 0;
		}
		return iter->second.belonging_error_category;
	}

	LUNA_RUNTIME_API Vector<errcat_t> get_all_error_categories()
	{
		LockGuard g(g_error_mtx);
		Vector<errcat_t> r;
		r.reserve(g_errcat_registry.get().size());
		for (auto& i : g_errcat_registry.get())
		{
			r.push_back(i.first);
		}
		return r;
	}

	LUNA_RUNTIME_API Vector<ErrCode> get_all_error_codes_of_category(errcat_t err_category)
	{
		LockGuard g(g_error_mtx);
		auto iter = g_errcat_registry.get().find(err_category);
		if (iter == g_errcat_registry.get().end())
		{
			return Vector<ErrCode>();
		}
		return iter->second.codes;
	}

	LUNA_RUNTIME_API Vector<errcat_t> get_all_error_subcategories_of_category(errcat_t err_category)
	{
		LockGuard g(g_error_mtx);
		auto iter = g_errcat_registry.get().find(err_category);
		if (iter == g_errcat_registry.get().end())
		{
			return Vector<errcat_t>();
		}
		return iter->second.subcategories;
	}

	LUNA_RUNTIME_API Error& get_error()
	{
		Error* err = (Error*)OS::tls_get(g_error_tls);
		if (!err)
		{
			err = memnew<Error>();
			OS::tls_set(g_error_tls, err);
		}
		return *err;
	}

	namespace BasicError
	{
		LUNA_RUNTIME_API errcat_t errtype()
		{
			static errcat_t e = get_error_category_by_name("BasicError");
			return e;
		}
		LUNA_RUNTIME_API ErrCode failure()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "failure");
			return e;
		}
		LUNA_RUNTIME_API ErrCode error_object()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "error_object");
			return e;
		}
		LUNA_RUNTIME_API ErrCode not_found()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "not_found");
			return e;
		}
		LUNA_RUNTIME_API ErrCode already_exists()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "already_exists");
			return e;
		}
		LUNA_RUNTIME_API ErrCode bad_arguments()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "bad_arguments");
			return e;
		}
		LUNA_RUNTIME_API ErrCode not_ready()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "not_ready");
			return e;
		}
		LUNA_RUNTIME_API ErrCode out_of_memory()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "out_of_memory");
			return e;
		}
		LUNA_RUNTIME_API ErrCode not_supported()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "not_supported");
			return e;
		}
		LUNA_RUNTIME_API ErrCode bad_platform_call()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "bad_platform_call");
			return e;
		}
		LUNA_RUNTIME_API ErrCode access_denied()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "access_denied");
			return e;
		}
		LUNA_RUNTIME_API ErrCode not_directory()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "not_directory");
			return e;
		}
		LUNA_RUNTIME_API ErrCode timeout()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "timeout");
			return e;
		}
		LUNA_RUNTIME_API ErrCode data_too_long()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "data_too_long");
			return e;
		}
		LUNA_RUNTIME_API ErrCode insufficient_user_buffer()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "insufficient_user_buffer");
			return e;
		}
		LUNA_RUNTIME_API ErrCode insufficient_buffer()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "insufficient_buffer");
			return e;
		}
		LUNA_RUNTIME_API ErrCode busy()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "busy");
			return e;
		}
		LUNA_RUNTIME_API ErrCode out_of_range()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "out_of_range");
			return e;
		}
		LUNA_RUNTIME_API ErrCode out_of_resource()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "out_of_resource");
			return e;
		}
		LUNA_RUNTIME_API ErrCode insufficient_system_buffer()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "insufficient_system_buffer");
			return e;
		}
		LUNA_RUNTIME_API ErrCode overflow()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "overflow");
			return e;
		}
		LUNA_RUNTIME_API ErrCode format_error()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "format_error");
			return e;
		}
		LUNA_RUNTIME_API ErrCode interrupted()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "interrupted");
			return e;
		}
		LUNA_RUNTIME_API ErrCode end_of_file()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "end_of_file");
			return e;
		}
		LUNA_RUNTIME_API ErrCode null_value()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "null_value");
			return e;
		}
		LUNA_RUNTIME_API ErrCode bad_cast()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "bad_cast");
			return e;
		}
		LUNA_RUNTIME_API ErrCode in_progress()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "in_progress");
			return e;
		}
		LUNA_RUNTIME_API ErrCode version_dismatch()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "version_dismatch");
			return e;
		}
		LUNA_RUNTIME_API ErrCode bad_data()
		{
			static ErrCode e = get_error_code_by_name("BasicError", "bad_data");
			return e;
		}
	}
}
