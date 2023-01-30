/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Log.hpp
* @author JXMaster
* @date 2022/6/30
*/
#include "Variant.hpp"
#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif
namespace Luna
{
	enum class LogVerbosity : u8
	{
		//! Unrecoverable error.
		fatal_error = 0,
		//! Recoverable error.
		error,
		//! Warnings.
		warning,
		//! Normal info.
		info,
		//! Verbose message that should not be displayed in normal condition.
		verbose,
	};

	struct LogMessage
	{
		//! The function/module that sends the log.
		Name sender;
		//! The log message.
		Name message;
		//! The verbosity of the log.
		LogVerbosity verbosity;
		//! Extra parameters attached with the log.
		Variant extra;
	};

	using log_callback_t = void(const LogMessage& message, void* userdata);

	LUNA_RUNTIME_API void log(const LogMessage& message);

	LUNA_RUNTIME_API void log_verbose(const Name& sender, const c8* format, ...);
	LUNA_RUNTIME_API void logv_verbose(const Name& sender, const c8* format, VarList args);
	LUNA_RUNTIME_API void log_info(const Name& sender, const c8* format, ...);
	LUNA_RUNTIME_API void logv_info(const Name& sender, const c8* format, VarList args);
	LUNA_RUNTIME_API void log_warning(const Name& sender, const c8* format, ...);
	LUNA_RUNTIME_API void logv_warning(const Name& sender, const c8* format, VarList args);
	LUNA_RUNTIME_API void log_error(const Name& sender, const c8* format, ...);
	LUNA_RUNTIME_API void logv_error(const Name& sender, const c8* format, VarList args);

	LUNA_RUNTIME_API void register_log_callback(log_callback_t* callback, void* userdata);
	LUNA_RUNTIME_API void unregister_log_callback(log_callback_t* callback);

	LUNA_RUNTIME_API void set_log_std_enabled(bool enabled);
	LUNA_RUNTIME_API void set_log_std_verbosity(LogVerbosity verbosity);

	LUNA_RUNTIME_API void set_log_file_enabled(bool enabled);
	LUNA_RUNTIME_API void set_log_file(const c8* file);
	LUNA_RUNTIME_API void set_log_file_verbosity(LogVerbosity verbosity);
}