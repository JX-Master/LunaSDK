/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Error.cpp
* @author JXMaster
* @date 2020/11/24
*/
#include <Luna/Runtime/PlatformDefines.hpp>

#define LUNA_RUNTIME_API LUNA_EXPORT
#include <Luna/Runtime/Error.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include "Platform/Fiber.hpp"
#include "../SpinLock.hpp"

namespace Luna
{
    SpinLock g_error_mtx;
    opaque_t g_error_tls;
    bool g_error_ready = false;

    struct ResultCodeRegistry
    {
        String name;
        String description;
        errcat_t category;
    };

    struct ErrCategoryRegistry
    {
        String name;
        Vector<ResultCode> codes;
    };

    Unconstructed<HashMap<ResultCode, ResultCodeRegistry>> g_result_code_registry;
    Unconstructed<HashMap<errcat_t, ErrCategoryRegistry>> g_errcat_registry;

    void error_destructor(void* cookie)
    {
        Error* err = static_cast<Error*>(cookie);
        memdelete(err);
    }

    LUNA_RUNTIME_API bool register_error_category(errcat_t category, const c8* name)
    {
        if(!g_error_ready || !name || !name[0] || (category >> 48)) return false;
        LockGuard guard(g_error_mtx);
        auto& categories = g_errcat_registry.get();
        auto existing = categories.find(category);
        if(existing != categories.end())
        {
            return !strcmp(existing->second.name.c_str(), name);
        }
        for(auto& entry : categories)
        {
            if(!strcmp(entry.second.name.c_str(), name)) return false;
        }
        categories.insert(make_pair(category, ErrCategoryRegistry{String(name), Vector<ResultCode>()}));
        return true;
    }

    LUNA_RUNTIME_API bool register_error_code(ResultCode code, const c8* name, const c8* description)
    {
        if(!g_error_ready || !code.code || !name || !name[0]) return false;
        if(!description) description = "";
        LockGuard guard(g_error_mtx);
        auto& categories = g_errcat_registry.get();
        const errcat_t category = get_error_code_category(code);
        auto category_iter = categories.find(category);
        if(category_iter == categories.end()) return false;

        auto& codes = g_result_code_registry.get();
        auto existing = codes.find(code);
        if(existing != codes.end())
        {
            return existing->second.category == category &&
                !strcmp(existing->second.name.c_str(), name) &&
                !strcmp(existing->second.description.c_str(), description);
        }
        for(ResultCode registered_code : category_iter->second.codes)
        {
            auto registered = codes.find(registered_code);
            if(registered != codes.end() && !strcmp(registered->second.name.c_str(), name)) return false;
        }
        codes.insert(make_pair(code, ResultCodeRegistry{String(name), String(description), category}));
        category_iter->second.codes.push_back(code);
        return true;
    }

    static bool register_runtime_error_codes()
    {
        if(!register_error_category(ERROR_CATEGORY, "Runtime")) return false;
        struct Info
        {
            ResultCode code;
            const c8* name;
            const c8* description;
        };
        constexpr Info infos[] =
        {
            {E_FAILURE, "failure", "General failure."},
            {E_ERROR_OBJECT, "error_object", "Detailed failure information is stored in the current thread's Error object."},
            {E_NOT_FOUND, "not_found", "The specified item does not exist."},
            {E_ALREADY_EXISTS, "already_exists", "The specified item already exists."},
            {E_NOT_UNIQUE, "not_unique", "The specified item is not unique."},
            {E_BAD_ARGUMENTS, "bad_arguments", "Invalid arguments were specified."},
            {E_BAD_CALLING_TIME, "bad_calling_time", "The function was called at an invalid time."},
            {E_OUT_OF_MEMORY, "out_of_memory", "The system cannot allocate enough memory."},
            {E_NOT_SUPPORTED, "not_supported", "The requested operation is not supported."},
            {E_BAD_PLATFORM_CALL, "bad_platform_call", "An underlying platform call failed."},
            {E_ACCESS_DENIED, "access_denied", "Access to the file or resource is denied."},
            {E_NOT_DIRECTORY, "not_directory", "The specified path is not a directory."},
            {E_IS_DIRECTORY, "is_directory", "The specified path is a directory."},
            {E_DIRECTORY_NOT_EMPTY, "directory_not_empty", "The directory is not empty."},
            {E_BAD_FILE, "bad_file", "The file format is invalid or unsupported."},
            {E_IO_ERROR, "io_error", "A system I/O operation failed."},
            {E_TIMEOUT, "timeout", "The operation did not complete before its deadline."},
            {E_DATA_TOO_BIG, "data_too_big", "The provided data or string is too large."},
            {E_INSUFFICIENT_USER_BUFFER, "insufficient_user_buffer", "The user-provided buffer is not large enough."},
            {E_NOT_READY, "not_ready", "The service provider is not ready."},
            {E_OUT_OF_RANGE, "out_of_range", "The provided value is outside its valid range."},
            {E_OUT_OF_RESOURCE, "out_of_resource", "The system has exhausted an internal resource."},
            {E_INSUFFICIENT_SYSTEM_BUFFER, "insufficient_system_buffer", "The system has exhausted an internal buffer."},
            {E_FORMAT_ERROR, "format_error", "A format error was detected."},
            {E_INTERRUPTED, "interrupted", "The operation was interrupted."},
            {E_END_OF_FILE, "end_of_file", "The end of a file or stream was reached."},
            {E_NULL_VALUE, "null_value", "An expected value is absent."},
            {E_BAD_CAST, "bad_cast", "A value does not conform to the requested type."},
            {E_IN_PROGRESS, "in_progress", "The operation is still in progress."},
            {E_VERSION_DISMATCH, "version_dismatch", "Library or platform versions do not match."},
            {E_NO_DATA, "no_data", "No data is available."},
            {E_BAD_DATA, "bad_data", "Data validation failed."},
            {E_BAD_MEMORY_ADDRESS, "bad_memory_address", "A memory address is invalid."},
            {E_DEADLOCK, "deadlock", "A deadlock was detected."},
            {E_NOT_PERMITTED, "not_permitted", "The requested operation is not permitted."},
            {E_BUSY, "busy", "The target device or service is busy."},
            {E_FILE_TOO_BIG, "file_too_big", "The file is too large."},
            {E_NOT_CONFIGURED, "not_configured", "The device or service is not configured."},
            {E_BAD_PIPE, "bad_pipe", "A POSIX pipe operation failed."},
            {E_PATH_TOO_LONG, "path_too_long", "The path is too long."},
            {E_LOOP, "loop", "A loop or circular reference was detected."},
        };
        for(const Info& info : infos)
        {
            if(!register_error_code(info.code, info.name, info.description)) return false;
        }
        return true;
    }

    bool error_init()
    {
        g_errcat_registry.construct();
        g_result_code_registry.construct();
        g_error_ready = true;
        auto result = Platform::fls_alloc(error_destructor, g_error_tls);
        if(result != Platform::Result::success || !register_runtime_error_codes())
        {
            if(result == Platform::Result::success) Platform::fls_free(g_error_tls);
            g_error_ready = false;
            g_result_code_registry.destruct();
            g_errcat_registry.destruct();
            return false;
        }
        return true;
    }

    void error_close()
    {
        Error* err = static_cast<Error*>(Platform::fls_get(g_error_tls));
        if(err)
        {
            memdelete(err);
            Platform::fls_set(g_error_tls, nullptr);
        }
        Platform::fls_free(g_error_tls);
        g_error_ready = false;
        g_result_code_registry.destruct();
        g_errcat_registry.destruct();
    }

    LUNA_RUNTIME_API const c8* get_error_code_name(ResultCode err_code)
    {
        if(!g_error_ready) return "";
        LockGuard guard(g_error_mtx);
        auto iter = g_result_code_registry.get().find(err_code);
        return iter == g_result_code_registry.get().end() ? "" : iter->second.name.c_str();
    }

    LUNA_RUNTIME_API const c8* get_error_code_description(ResultCode err_code)
    {
        if(!g_error_ready) return "";
        LockGuard guard(g_error_mtx);
        auto iter = g_result_code_registry.get().find(err_code);
        return iter == g_result_code_registry.get().end() ? "" : iter->second.description.c_str();
    }

    LUNA_RUNTIME_API const c8* get_error_category_name(errcat_t err_category)
    {
        if(!g_error_ready) return "";
        LockGuard guard(g_error_mtx);
        auto iter = g_errcat_registry.get().find(err_category);
        return iter == g_errcat_registry.get().end() ? "" : iter->second.name.c_str();
    }

    LUNA_RUNTIME_API Vector<errcat_t> get_all_error_categories()
    {
        Vector<errcat_t> result;
        if(!g_error_ready) return result;
        LockGuard guard(g_error_mtx);
        result.reserve(g_errcat_registry.get().size());
        for(auto& entry : g_errcat_registry.get()) result.push_back(entry.first);
        return result;
    }

    LUNA_RUNTIME_API Vector<ResultCode> get_all_error_codes_of_category(errcat_t err_category)
    {
        if(!g_error_ready) return Vector<ResultCode>();
        LockGuard guard(g_error_mtx);
        auto iter = g_errcat_registry.get().find(err_category);
        return iter == g_errcat_registry.get().end() ? Vector<ResultCode>() : iter->second.codes;
    }

    LUNA_RUNTIME_API Error& get_error()
    {
        Error* err = static_cast<Error*>(Platform::fls_get(g_error_tls));
        if(!err)
        {
            err = memnew<Error>();
            Platform::fls_set(g_error_tls, err);
        }
        return *err;
    }

}
