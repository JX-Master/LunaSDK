/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Error.hpp
* @author JXMaster
* @date 2021/4/19
*/
#pragma once

#include "Base.hpp"
#include "Assert.hpp"
#include "String.hpp"
#include "Vector.hpp"
#include "Variant.hpp"

#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif

namespace Luna
{
    //! @addtogroup Runtime
    //! @{
    //! @defgroup RuntimeError Error handling
    //! @}

    //! @addtogroup RuntimeError
    //! @{
    
    //! Identifies one operation result.
    //! @details The high 32 bits identify the error domain, the following 16 bits identify the
    //! category in that domain, and the low 16 bits contain a signed result value. Negative result
    //! values indicate failure. Non-negative result values indicate success. The all-zero value is
    //! the only plain success result; every other successful value carries status information.
    struct ResultCode
    {
        //! The stable 64-bit result code.
        u64 code;

        constexpr ResultCode() : code(0) {}
        explicit constexpr ResultCode(u64 code) : code(code) {}
        constexpr ResultCode(const ResultCode&) = default;
        constexpr ResultCode(ResultCode&&) = default;
        constexpr ResultCode& operator=(const ResultCode&) = default;
        constexpr ResultCode& operator=(ResultCode&&) = default;
        constexpr bool operator==(const ResultCode& rhs) const { return code == rhs.code; }
        constexpr bool operator!=(const ResultCode& rhs) const { return code != rhs.code; }
        constexpr bool operator>(const ResultCode& rhs) const { return code > rhs.code; }
        constexpr bool operator<(const ResultCode& rhs) const { return code < rhs.code; }
        constexpr bool operator>=(const ResultCode& rhs) const { return code >= rhs.code; }
        constexpr bool operator<=(const ResultCode& rhs) const { return code <= rhs.code; }
    };

    template<>
    struct hash<ResultCode>
    {
        usize operator()(ResultCode val) const { return static_cast<usize>(val.code); }
    };

    //! Constructs a stable result code.
    //! @param[in] domain_id The 32-bit domain identifier.
    //! @param[in] category_id The 16-bit category identifier allocated by the domain.
    //! @param[in] result The signed result value. Negative values indicate failure.
    //! @return Returns the constructed result code.
    constexpr ResultCode make_error_code(u32 domain_id, u16 category_id, i16 result)
    {
        return ResultCode(
            (static_cast<u64>(domain_id) << 32) |
            (static_cast<u64>(category_id) << 16) |
            static_cast<u16>(result));
    }

    //! Gets the domain identifier encoded in the result code.
    constexpr u32 get_error_code_domain(ResultCode code)
    {
        return static_cast<u32>(code.code >> 32);
    }

    //! Gets the category identifier encoded in the result code.
    constexpr u16 get_error_code_category_id(ResultCode code)
    {
        return static_cast<u16>((code.code >> 16) & 0xFFFF);
    }

    //! Gets the signed result value encoded in the result code.
    constexpr i16 get_error_code_result(ResultCode code)
    {
        const u32 raw = static_cast<u32>(code.code & 0xFFFF);
        const i32 value = raw < 0x8000 ? static_cast<i32>(raw) : static_cast<i32>(raw) - 0x10000;
        return static_cast<i16>(value);
    }

    //! Tests whether the result code indicates success.
    constexpr bool succeeded(ResultCode code)
    {
        return get_error_code_result(code) >= 0;
    }

    //! Tests whether the result code indicates failure.
    constexpr bool failed(ResultCode code)
    {
        return get_error_code_result(code) < 0;
    }

    //! Tests whether the result is the all-zero plain success result.
    constexpr bool is_plain_success(ResultCode code)
    {
        return code.code == 0;
    }

    //! Tests whether the result indicates success with status information.
    constexpr bool is_informative_success(ResultCode code)
    {
        return code.code != 0 && succeeded(code);
    }

    //! The greatest domain ID in the centrally registered region.
    inline constexpr u32 REGISTERED_ERROR_DOMAIN_MAX = 0x7FFFFFFF;
    //! The first domain ID in the self-allocated region.
    inline constexpr u32 SELF_ALLOCATED_ERROR_DOMAIN_MIN = 0x80000000;

    //! Tests whether a domain ID belongs to the centrally registered region.
    constexpr bool is_registered_error_domain(u32 domain_id)
    {
        return domain_id <= REGISTERED_ERROR_DOMAIN_MAX;
    }

    //! Tests whether a domain ID belongs to the self-allocated region.
    constexpr bool is_self_allocated_error_domain(u32 domain_id)
    {
        return domain_id >= SELF_ALLOCATED_ERROR_DOMAIN_MIN;
    }

    //! The stable error category identifier. Bits 16-47 store the domain ID and bits 0-15
    //! store the category ID. The upper 16 bits are always zero.
    using errcat_t = u64;

    //! The reserved invalid error category value.
    constexpr errcat_t INVALID_ERROR_CATEGORY = U64_MAX;

    //! Constructs one stable error category identifier.
    constexpr errcat_t make_error_category(u32 domain_id, u16 category_id)
    {
        return (static_cast<u64>(domain_id) << 16) | static_cast<u64>(category_id);
    }

    //! Gets the domain identifier of an error category.
    constexpr u32 get_error_category_domain(errcat_t category)
    {
        return static_cast<u32>((category >> 16) & 0xFFFFFFFF);
    }

    //! Gets the category identifier of an error category.
    constexpr u16 get_error_category_id(errcat_t category)
    {
        return static_cast<u16>(category & 0xFFFF);
    }

    //! Gets the category that owns the specified result code.
    constexpr errcat_t get_error_code_category(ResultCode code)
    {
        return make_error_category(get_error_code_domain(code), get_error_code_category_id(code));
    }

    //! Registers static information for an error category.
    //! @param[in] category The stable category identifier.
    //! @param[in] name The globally recognizable category name.
    //! @return Returns `true` if the category was registered, or was already registered with the
    //! same name. Returns `false` if the identifier or name conflicts with an existing category.
    //! @remark This function requires Luna Runtime to be initialized.
    LUNA_RUNTIME_API bool register_error_category(errcat_t category, const c8* name);

    //! Registers static information for one result code.
    //! @param[in] code The stable result code to register. The all-zero plain success result cannot
    //! be registered.
    //! @param[in] name The result name, unique in its category.
    //! @param[in] description A brief description of the result.
    //! @return Returns `true` if the information was registered, or was already registered with the
    //! same values. Returns `false` if the code or name conflicts with existing information.
    //! @remark This function requires Luna Runtime to be initialized.
    LUNA_RUNTIME_API bool register_error_code(ResultCode code, const c8* name, const c8* description);

    //! Fetches the diagnostic name of the result code.
    //! @param[in] err_code The result code value.
    //! @return Returns the name of the result code. This string is valid until the
    //! runtime is closed. Returns empty string ("") if the result code does not exist.
    LUNA_RUNTIME_API const c8* get_error_code_name(ResultCode err_code);

    //! Fetches the brief description of one registered result code.
    //! @param[in] err_code The result code value.
    //! @return Returns the registered description, or an empty string if no information is registered.
    LUNA_RUNTIME_API const c8* get_error_code_description(ResultCode err_code);

    //! Fetches the name of the error category.
    //! @param[in] err_category The error category value.
    //! @return Returns the name of the error category. This string is valid until the 
    //! runtime is closed. Returns empty string ("") if the error category does not exist.
    LUNA_RUNTIME_API const c8* get_error_category_name(errcat_t err_category);

    //! Fetches all error categories registered in the system.
    //! @return Returns all error categories registered in the system.
    LUNA_RUNTIME_API Vector<errcat_t> get_all_error_categories();

    //! Fetches all registered result codes that belong to the specified error category.
    //! @param[in] err_category The error category to look up.
    //! @return Returns all registered result codes that belong to the specified category.
    LUNA_RUNTIME_API Vector<ResultCode> get_all_error_codes_of_category(errcat_t err_category);

    //! The error object encapsulates one failure result code along with one
    //! string that describes the error.
    struct Error
    {
        ResultCode code;
        String message;
        Variant info;
        Error()
        {
            reset();
        }
        Error(ResultCode code, const String& message) :
            code(code),
            message(message) {}
        Error(ResultCode code, const c8* fmt, VarList args) :
            code(code)
        {
            c8 buf[1024];
            vsnprintf(buf, 1024, fmt, args);
            message = buf;
        }
        Error(ResultCode code, const c8* fmt, ...) :
            code(code)
        {
            VarList args;
            va_start(args, fmt);
            c8 buf[1024];
            vsnprintf(buf, 1024, fmt, args);
            message = buf;
            va_end(args);
        }
        Error(const Error& rhs) = default;
        Error(Error&& rhs) = default;
        Error& operator=(const Error& rhs) = default;
        Error& operator=(Error&& rhs) = default;
        void reset()
        {
            code = ResultCode(0);
            message = "";
            info = Variant();
        }
        const c8* explain() const
        {
            if (!message.empty())
            {
                return message.c_str();
            }
            const c8* description = get_error_code_description(code);
            return description[0] ? description : get_error_code_name(code);
        }
    };

    //! Gets the error object of this thread. Every thread will be assigned with one error object.
    //! @return Returns the error object of this thread.
    LUNA_RUNTIME_API Error& get_error();

    //! Registered error domain identifiers used by LunaSDK.
    namespace ErrorDomain
    {
        //! The LunaSDK error domain.
        inline constexpr u32 LUNA_SDK = 0;
    }

    //! Category identifiers allocated by the LunaSDK error domain.
    namespace LunaErrorCategory
    {
        //! The Runtime module.
        inline constexpr u16 RUNTIME = 0;
        //! The AHI module.
        inline constexpr u16 AHI = 1;
        //! The Asset module.
        inline constexpr u16 ASSET = 2;
        //! The ECS module.
        inline constexpr u16 ECS = 3;
        //! The Frontend module.
        inline constexpr u16 FRONTEND = 4;
        //! The Image module.
        inline constexpr u16 IMAGE = 5;
        //! The Network module.
        inline constexpr u16 NETWORK = 6;
        //! The RHI module.
        inline constexpr u16 RHI = 7;
        //! The VFS module.
        inline constexpr u16 VFS = 8;
    }

    //! @addtogroup RuntimeError
    //! @{

    //! @name Runtime result codes
    //! Contains common result codes defined by the Runtime module. These codes can also be used by other modules.
    //! @{

    //! The Runtime error category identifier.
    inline constexpr errcat_t ERROR_CATEGORY = make_error_category(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME);

    //! General failure.
    inline constexpr ResultCode E_FAILURE = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -1);
    //! The real error is recorded in the error object of the current thread.
    inline constexpr ResultCode E_ERROR_OBJECT = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -2);
    //! The specified item does not exist.
    inline constexpr ResultCode E_NOT_FOUND = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -3);
    //! The specified item already exists.
    inline constexpr ResultCode E_ALREADY_EXISTS = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -4);
    //! The specified item is not unique.
    inline constexpr ResultCode E_NOT_UNIQUE = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -5);
    //! Invalid arguments are specified.
    inline constexpr ResultCode E_BAD_ARGUMENTS = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -6);
    //! The function is called at an invalid time.
    inline constexpr ResultCode E_BAD_CALLING_TIME = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -7);
    //! The system cannot allocate enough memory.
    inline constexpr ResultCode E_OUT_OF_MEMORY = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -8);
    //! The requested operation is not supported.
    inline constexpr ResultCode E_NOT_SUPPORTED = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -9);
    //! An underlying platform call failed.
    inline constexpr ResultCode E_BAD_PLATFORM_CALL = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -10);
    //! Access to a file or resource is denied.
    inline constexpr ResultCode E_ACCESS_DENIED = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -11);
    //! The specified path is not a directory.
    inline constexpr ResultCode E_NOT_DIRECTORY = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -12);
    //! The specified path is a directory.
    inline constexpr ResultCode E_IS_DIRECTORY = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -13);
    //! The directory is not empty.
    inline constexpr ResultCode E_DIRECTORY_NOT_EMPTY = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -14);
    //! The file format is invalid or unsupported.
    inline constexpr ResultCode E_BAD_FILE = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -15);
    //! A system I/O operation failed.
    inline constexpr ResultCode E_IO_ERROR = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -16);
    //! The operation did not complete before its deadline.
    inline constexpr ResultCode E_TIMEOUT = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -17);
    //! The provided data or string is too large.
    inline constexpr ResultCode E_DATA_TOO_BIG = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -18);
    //! The user-provided buffer is not large enough.
    inline constexpr ResultCode E_INSUFFICIENT_USER_BUFFER = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -19);
    //! The service provider is not ready.
    inline constexpr ResultCode E_NOT_READY = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -20);
    //! The provided value is outside its valid range.
    inline constexpr ResultCode E_OUT_OF_RANGE = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -21);
    //! The system has exhausted an internal resource.
    inline constexpr ResultCode E_OUT_OF_RESOURCE = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -22);
    //! The system has exhausted an internal buffer.
    inline constexpr ResultCode E_INSUFFICIENT_SYSTEM_BUFFER = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -23);
    //! A format error was detected.
    inline constexpr ResultCode E_FORMAT_ERROR = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -24);
    //! The operation was interrupted.
    inline constexpr ResultCode E_INTERRUPTED = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -25);
    //! The end of a file or stream was reached.
    inline constexpr ResultCode E_END_OF_FILE = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -26);
    //! An expected value is absent.
    inline constexpr ResultCode E_NULL_VALUE = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -27);
    //! A value does not conform to the requested type.
    inline constexpr ResultCode E_BAD_CAST = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -28);
    //! The operation is still in progress and is treated as a failure by its existing APIs.
    inline constexpr ResultCode E_IN_PROGRESS = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -29);
    //! Library or platform versions do not match.
    inline constexpr ResultCode E_VERSION_DISMATCH = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -30);
    //! No data is available.
    inline constexpr ResultCode E_NO_DATA = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -31);
    //! Data validation failed.
    inline constexpr ResultCode E_BAD_DATA = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -32);
    //! A memory address is invalid.
    inline constexpr ResultCode E_BAD_MEMORY_ADDRESS = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -33);
    //! A deadlock was detected.
    inline constexpr ResultCode E_DEADLOCK = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -34);
    //! The requested operation is not permitted.
    inline constexpr ResultCode E_NOT_PERMITTED = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -35);
    //! The target device or service is busy.
    inline constexpr ResultCode E_BUSY = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -36);
    //! The file is too large.
    inline constexpr ResultCode E_FILE_TOO_BIG = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -37);
    //! The device or service is not configured.
    inline constexpr ResultCode E_NOT_CONFIGURED = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -38);
    //! A POSIX pipe operation failed.
    inline constexpr ResultCode E_BAD_PIPE = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -39);
    //! The path is too long.
    inline constexpr ResultCode E_PATH_TOO_LONG = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -40);
    //! A loop or circular reference was detected.
    inline constexpr ResultCode E_LOOP = make_error_code(ErrorDomain::LUNA_SDK, LunaErrorCategory::RUNTIME, -41);

    //! @}

    //! @}

    //! Sets the error object of this thread.
    //! @details This function is auxiliary and it behaves the same as fetching the error object then setting it.
    //! @param[in] code The failure result code to set.
    //! @param[in] fmt The error message formatting syntax.
    //! @param[in] args The arguments for the error message.
    //! @return Returns @ref E_ERROR_OBJECT always, so you may chain this function with the return statement like `return set_error(...)`.
    inline ResultCode set_error_v(ResultCode code, const c8* fmt, VarList args)
    {
        luassert(failed(code));
        get_error() = Error(code, fmt, args);
        return E_ERROR_OBJECT;
    }

    //! Sets the error object of this thread.
    //! @details This function is auxiliary and it behaves the same as fetching the error object then setting it.
    //! @param[in] code The failure result code to set.
    //! @param[in] fmt The error message formatting syntax.
    //! @param[in] args The arguments for the error message.
    //! @return Returns @ref E_ERROR_OBJECT always, so you may chain this function with the return statement like `return set_error(...)`.
    inline ResultCode set_error(ResultCode code, const c8* fmt, ...)
    {
        luassert(failed(code));
        VarList args;
        va_start(args, fmt);
        get_error() = Error(code, fmt, args);
        va_end(args);
        return E_ERROR_OBJECT;
    }

    //! Gets a brief description of the result code.
    //! @param[in] err_code The result code to fetch.
    //! @return Returns one C string that describes the result code.
    //! @remark 
    //! If `err_code` equals to @ref E_ERROR_OBJECT, the returned string will be fetched from `get_error().message.c_str(),
    //! otherwise, the registered description will be returned when available, followed by the
    //! registered result name as a fallback.
    inline const c8* explain(ResultCode err_code)
    {
        if (err_code == E_ERROR_OBJECT)
        {
            return get_error().message.c_str();
        }
        const c8* description = get_error_code_description(err_code);
        return description[0] ? description : get_error_code_name(err_code);
    }

    //! Gets the real failure result code if the result code is @ref E_ERROR_OBJECT.
    //! @param[in] err_code The result code to unwrap.
    //! @return If the result code is @ref E_ERROR_OBJECT, returns `get_error().code`. Otherwise, returns `err_code` directly.
    inline ResultCode unwrap_errcode(ResultCode err_code)
    {
        if (err_code == E_ERROR_OBJECT)
        {
            return get_error().code;
        }
        return err_code;
    }

    //! @}
}
