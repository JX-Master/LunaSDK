/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Error.hpp
* @author JXMaster
* @date 2021/4/19
*/
#pragma once

#include "Base.hpp"
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
	
	//! The error code type represents one single error. 
	//! @details In Luna Runtime, every error is represented by one error code, 
	//! the code value will be determined when the error is firstly accessed, and will never be changed.
	//! 
	//! Any valid error code number will not be 0.
	//! 
	//! The error code is wrapped in a unique structure so that it will never be misinterpreted 
	//! to a normal return value when the function return value is also an integer.
	struct ErrCode
	{
		//! The identifier of the error code.
		usize code;

		ErrCode() = default;
		explicit constexpr ErrCode(usize code) :code(code) {}
		ErrCode(const ErrCode&) = default;
		ErrCode(ErrCode&&) = default;
		ErrCode& operator=(const ErrCode&) = default;
		ErrCode& operator=(ErrCode&&) = default;
		constexpr bool operator==(const ErrCode& rhs) const { return code == rhs.code; }
		constexpr bool operator!=(const ErrCode& rhs) const { return code != rhs.code; }
		constexpr bool operator>(const ErrCode& rhs) const { return code > rhs.code; }
		constexpr bool operator<(const ErrCode& rhs) const { return code < rhs.code; }
		constexpr bool operator>=(const ErrCode& rhs) const { return code >= rhs.code; }
		constexpr bool operator<=(const ErrCode& rhs) const { return code <= rhs.code; }
	};

	template<>
	struct hash<ErrCode>
	{
		usize operator()(ErrCode val) const { return val.code; }
	};

	//! The error category type represents one container that can hold multiple error
	//! codes and sub-categories. 
	//! @details Like the error code, the category value will be determined when the error 
	//! category is firstly accessed, and will never be changed.
	//! 
	//! Any valid error type number will not be 0.
	using errcat_t = usize;

	//! Gets the error code represented by the error name.
	//! @param[in] errcat_name The name of the category that holds the error name.
	//! Use "::" to separate category names if the category is a sub-category.
	//! @param[in] errcode_name The name of the error code.
	//! @return Returns the error code of the corresponding error name. The return value will never be 
	//! 0, if the error code name is required for the first time, the system creates one new registry 
	//! for the error code, and returns one valid error code number that represents to the error.
	LUNA_RUNTIME_API ErrCode get_error_code_by_name(const c8* errcat_name, const c8* errcode_name);

	//! Gets the error category represented by the error category name.
	//! @param[in] errcat_name The full name of the error category.
	//! Use "::" to separate category names if the category is a sub-category.
	//! @return Returns the error category of the corresponding error name. The return value will never be 
	//! 0, if the error category name is required for the first time, the system creates one new registry 
	//! for the error category, and returns one valid error category number that represents to the error category.
	LUNA_RUNTIME_API errcat_t get_error_category_by_name(const c8* errcat_name);

	//! Fetches the name of the error code.
	//! @param[in] err_code The error code value.
	//! @return Returns the name of the error code. This string is valid until the 
	//! runtime is closed. Returns empty string ("") if the error code does not exist.
	LUNA_RUNTIME_API const c8* get_error_code_name(ErrCode err_code);

	//! Fetches the name of the error category.
	//! @param[in] err_category The error category value.
	//! @return Returns the name of the error category. This string is valid until the 
	//! runtime is closed. Returns empty string ("") if the error category does not exist.
	LUNA_RUNTIME_API const c8* get_error_category_name(errcat_t err_category);

	//! Fetches the error category that holds the error code.
	//! @param[in] err_code The error code value.
	//! @return Returns the error category that holds the error code, or 0 if the error 
	//! code does not exist.
	LUNA_RUNTIME_API errcat_t get_error_code_category(ErrCode err_code);

	//! Fetches all error categories registered in the system, including all subcategories.
	//! @return Returns all error categories registered in the system.
	LUNA_RUNTIME_API Vector<errcat_t> get_all_error_categories();

	//! Fetches all error codes that belongs to the specified error category.
	//! @param[in] err_category The error category to look up.
	//! @return Returns all error codes that belongs to the specified category.
	LUNA_RUNTIME_API Vector<ErrCode> get_all_error_codes_of_category(errcat_t err_category);

	//! Fetches all child error categories that belongs to the specified error category.
	//! @param[in] err_category The error category to look up.
	//! @return Returns all child error categories that belongs to the specified category.
	LUNA_RUNTIME_API Vector<errcat_t> get_all_error_subcategories_of_category(errcat_t err_category);

	//! The error object encapsulates one error code along with one
	//! string that describes the error.
	struct Error
	{
		ErrCode code;
		String message;
		Variant info;
		Error()
		{
			reset();
		}
		Error(ErrCode code, const String& message) :
			code(code),
			message(message) {}
		Error(ErrCode code, const c8* fmt, VarList args) :
			code(code)
		{
			c8 buf[1024];
			vsnprintf(buf, 1024, fmt, args);
			message = buf;
		}
		Error(ErrCode code, const c8* fmt, ...) :
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
			code = ErrCode(0);
			message = "";
			info = Variant();
		}
		const c8* explain() const
		{
			if (!message.empty())
			{
				return message.c_str();
			}
			return get_error_code_name(code);
		}
	};

	//! Gets the error object of this thread. Every thread will be assigned with one error object.
	//! @return Returns the error object of this thread.
	LUNA_RUNTIME_API Error& get_error();

	namespace BasicError
	{
		//! @addtogroup RuntimeError
   		//! @{

		//! @name Basic errors
		//! Contains core errors defined by Luna SDK. These errors can also be used by user modules.
		//! @{
		
		//! Gets the error category object of `BasicError`.
		LUNA_RUNTIME_API errcat_t errtype();
		//! General failure. 
		//! @remark The user should avoid returning generic failure if it can determine what is going wrong and can report that
		//! since returning generic failure does not provide any information for the caller to handle the error.
		LUNA_RUNTIME_API ErrCode failure();
		//! The error is recorded in the error object of the current thread. The user should call `get_error()` to fetch the error
		//! object to check the real error.
		LUNA_RUNTIME_API ErrCode error_object();
		//! The specified item does not exist.
		LUNA_RUNTIME_API ErrCode not_found();
		//! The specified item already exists.
		LUNA_RUNTIME_API ErrCode already_exists();
		//! The specified item is not unique.
		LUNA_RUNTIME_API ErrCode not_unique();
		//! Invalid arguments are specified. This is caused by a programming error and must be fixed before the application is released.
		LUNA_RUNTIME_API ErrCode bad_arguments();
		//! The function calling time is not valid, like using one resource before it is initialized, or 
		//! trying to reset one resource when the resource is still using.
		//! This is caused by a programming error and must be fixed before the application is released.
		LUNA_RUNTIME_API ErrCode bad_calling_time();
		//! The system cannot allocate enough memory to finish this operation.
		LUNA_RUNTIME_API ErrCode out_of_memory();
		//! The required operation is not supported by the instance/platform/build.
		LUNA_RUNTIME_API ErrCode not_supported();
		//! A call to the underlying OS/platform/library was failed and the reason cannot be represented by any other error code.
		LUNA_RUNTIME_API ErrCode bad_platform_call();
		//! The access to the file or resource is denied.
		LUNA_RUNTIME_API ErrCode access_denied();
		//! The specified path is not a directory, or one component of the path prefix of the specified path is not a directory.
		LUNA_RUNTIME_API ErrCode not_directory();
		//! The specified path is a directory.
		LUNA_RUNTIME_API ErrCode is_directory();
		//! The directory is not empty.
		LUNA_RUNTIME_API ErrCode directory_not_empty();
		//! The file format is not valid or not supported.
		LUNA_RUNTIME_API ErrCode bad_file();
		//! System IO error.
		LUNA_RUNTIME_API ErrCode io_error();
		//! The time limit is reached before this operation succeeds.
		LUNA_RUNTIME_API ErrCode timeout();
		//! The provided data or string is too long.
		LUNA_RUNTIME_API ErrCode data_too_long();
		//! The buffer provided by the user is not large enough to contain all returned data.
		LUNA_RUNTIME_API ErrCode insufficient_user_buffer();
		//! The service provider is not ready to handle this call at this moment.
		//! The user should try again later.
		LUNA_RUNTIME_API ErrCode not_ready();
		//! The value specified by the user is out of the valid range from the value.
		LUNA_RUNTIME_API ErrCode out_of_range();
		//! The system has run out of intenal resources (usually handles or resources with a fixed count limit) to finish the operation.
		LUNA_RUNTIME_API ErrCode out_of_resource();
		//! The system has run out of internal buffer to finish the operation.
		//! This usually indicates that one data or message buffer/queue is full.
		LUNA_RUNTIME_API ErrCode insufficient_system_buffer();
		//! Format error detected when parsing the scripts or strings.
		LUNA_RUNTIME_API ErrCode format_error();
		//! The operation is interrupted by user or system.
		LUNA_RUNTIME_API ErrCode interrupted();
		//! The end of the file or data stream is reached.
		LUNA_RUNTIME_API ErrCode end_of_file();
		//! One instance is expected in the context, but the value in one null-able type is absent.
		LUNA_RUNTIME_API ErrCode null_value();
		//! The instance provided to the as expression does not conforms to the type provided.
		LUNA_RUNTIME_API ErrCode bad_cast();
		//! The operation is still in progress when this call returns. This is usually not an error, but a notation to the user that
		//! the operation is asynchronous.
		LUNA_RUNTIME_API ErrCode in_progress();
		//! The library/platform version is not matched.
		LUNA_RUNTIME_API ErrCode version_dismatch();
		//! No data is available.
		LUNA_RUNTIME_API ErrCode no_data();
		//! The data validation is failed.
		LUNA_RUNTIME_API ErrCode bad_data();
		//! The address is not valid.
		LUNA_RUNTIME_API ErrCode bad_address();
		//! One deadlock is detected.
		LUNA_RUNTIME_API ErrCode deadlock();

		//! @}

		//! @}
	}

	//! Sets the error object of this thread.
	//! @details This function is auxiliary and it behaves the same as fetching the error object then setting it.
	//! @param[in] code The error code to set.
	//! @param[in] fmt The error message formatting syntax.
	//! @param[in] args The arguments for the error message.
	//! @return Returns @ref BasicError::error_object always, so you may chain this function with the return statement like `return set_error(...)`.
	inline ErrCode set_error(ErrCode code, const c8* fmt, VarList args)
	{
		get_error() = Error(code, fmt, args);
		return BasicError::error_object();
	}

	//! Sets the error object of this thread.
	//! @details This function is auxiliary and it behaves the same as fetching the error object then setting it.
	//! @param[in] code The error code to set.
	//! @param[in] fmt The error message formatting syntax.
	//! @param[in] args The arguments for the error message.
	//! @return Returns @ref BasicError::error_object always, so you may chain this function with the return statement like `return set_error(...)`.
	inline ErrCode set_error(ErrCode code, const c8* fmt, ...)
	{
		VarList args;
		va_start(args, fmt);
		get_error() = Error(code, fmt, args);
		va_end(args);
		return BasicError::error_object();
	}

	//! Gets a brief description about the error code.
	//! @param[in] err_code The error code to fetch.
	//! @return Returns one C string that describes the error code.
	//! @remark 
	//! If `err_code` equals to @ref BasicError::error_object, the returned string will be fetched from `get_error().message.c_str(),
	//! otherwise, the returned string will be fetched from `get_error_code_name(err_code)`.
	inline const c8* explain(ErrCode err_code)
	{
		if (err_code == BasicError::error_object())
		{
			return get_error().message.c_str();
		}
		return get_error_code_name(err_code);
	}

	//! Gets the real error code if the error code is @ref BasicError::error_object.
	//! @param[in] err_code The error code to unwrap.
	//! @return If the error code is @ref BasicError::error_object, returns `get_error().code`. Otherwise, returns `err_code` directly.
	inline ErrCode unwrap_errcode(ErrCode err_code)
	{
		if (err_code == BasicError::error_object())
		{
			return get_error().code;
		}
		return err_code;
	}

	//! @}
}