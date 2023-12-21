/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Log.hpp
* @author JXMaster
* @date 2022/6/30
*/
#pragma once
#include "Variant.hpp"
#include "Event.hpp"
#ifndef LUNA_RUNTIME_API
#define LUNA_RUNTIME_API
#endif
namespace Luna
{
	//! @addtogroup Runtime
	//! @{
	//! @defgroup RuntimeLog Logging
	//! @}

	//! @addtogroup RuntimeLog
    //! @{
	
	//! Defines all log verbosity levels.
	enum class LogVerbosity : u8
	{
		//! Unrecoverable error.
		fatal_error = 0,
		//! Recoverable error.
		error = 1,
		//! Warnings.
		warning = 2,
		//! Normal info.
		info = 3,
		//! Debug info.
		debug = 4,
		//! Verbose message that should not be displayed in normal condition.
		verbose = 5,
	};

	//! Called by the log system when one log is emitted.
	//! @param[in] verbosity The log verbosity.
	//! @param[in] tag The log tag. Used by the implementation to filter logs.
	//! @param[in] tag_len The length of the tag string.
	//! @param[in] message The log message.
	//! @param[in] message_length The message length.
	using log_callback_t = void(LogVerbosity verbosity, const c8* tag, usize tag_length, const c8* message, usize message_length);

	//! Logs one message.
	//! @param[in] verbosity The log verbosity.
	//! @param[in] tag The log tag. Used by the implementation to filter logs.
	//! @param[in] format The log message format.
	LUNA_RUNTIME_API void log(LogVerbosity verbosity, const c8* tag, const c8* format, ...);

	//! Logs one message.
	//! @param[in] verbosity The log verbosity.
	//! @param[in] tag The log tag. Used by the implementation to filter logs.
	//! @param[in] format The log message format.
	//! @param[in] args Arguments used to format the log message.
	LUNA_RUNTIME_API void logv(LogVerbosity verbosity, const c8* tag, const c8* format, VarList args);
	
	//! Outputs one log message with @ref LogVerbosity::verbose verbosity.
	//! @param[in] tag The log tag. Used by the implementation to filter logs.
	//! @param[in] format The formatting syntax used to format the log message.
	LUNA_RUNTIME_API void log_verbose(const c8* tag, const c8* format, ...);
	//! Outputs one log message with @ref LogVerbosity::verbose verbosity.
	//! @param[in] tag The log tag. Used by the implementation to filter logs.
	//! @param[in] format The formatting syntax used to format the log message.
	//! @param[in] args Arguments used to format the log message.
	LUNA_RUNTIME_API void logv_verbose(const c8* tag, const c8* format, VarList args);
	//! Outputs one log message with @ref LogVerbosity::debug verbosity.
	//! @param[in] tag The log tag. Used by the implementation to filter logs.
	//! @param[in] format The formatting syntax used to format the log message.
	LUNA_RUNTIME_API void log_debug(const c8* tag, const c8* format, ...);
	//! Outputs one log message with @ref LogVerbosity::debug verbosity.
	//! @param[in] tag The log tag. Used by the implementation to filter logs.
	//! @param[in] format The formatting syntax used to format the log message.
	//! @param[in] args Arguments used to format the log message.
	LUNA_RUNTIME_API void logv_debug(const c8* tag, const c8* format, VarList args);
	//! Outputs one log message with @ref LogVerbosity::info verbosity.
	//! @param[in] tag The log tag. Used by the implementation to filter logs.
	//! @param[in] format The formatting syntax used to format the log message.
	LUNA_RUNTIME_API void log_info(const c8* tag, const c8* format, ...);
	//! Outputs one log message with @ref LogVerbosity::info verbosity.
	//! @param[in] tag The log tag. Used by the implementation to filter logs.
	//! @param[in] format The formatting syntax used to format the log message.
	//! @param[in] args Arguments used to format the log message.
	LUNA_RUNTIME_API void logv_info(const c8* tag, const c8* format, VarList args);
	//! Outputs one log message with @ref LogVerbosity::warning verbosity.
	//! @param[in] tag The log tag. Used by the implementation to filter logs.
	//! @param[in] format The formatting syntax used to format the log message.
	LUNA_RUNTIME_API void log_warning(const c8* tag, const c8* format, ...);
	//! Outputs one log message with @ref LogVerbosity::warning verbosity.
	//! @param[in] tag The log tag. Used by the implementation to filter logs.
	//! @param[in] format The formatting syntax used to format the log message.
	//! @param[in] args Arguments used to format the log message.
	LUNA_RUNTIME_API void logv_warning(const c8* tag, const c8* format, VarList args);
	//! Outputs one log message with @ref LogVerbosity::error verbosity.
	//! @param[in] tag The log tag. Used by the implementation to filter logs.
	//! @param[in] format The formatting syntax used to format the log message.
	LUNA_RUNTIME_API void log_error(const c8* tag, const c8* format, ...);
	//! Outputs one log message with @ref LogVerbosity::error verbosity.
	//! @param[in] tag The log tag. Used by the implementation to filter logs.
	//! @param[in] format The formatting syntax used to format the log message.
	//! @param[in] args Arguments used to format the log message.
	LUNA_RUNTIME_API void logv_error(const c8* tag, const c8* format, VarList args);

	//! Registers one custom log handler that will be called when a new log message is spawned.
	//! @param[in] handler The handler to register.
	//! @return Returns one handler identifier that can be used to register the handler.
	LUNA_RUNTIME_API usize register_log_handler(const Function<log_callback_t>& handler);
	//! Unregisters one registered log handler.
	//! @param[in] handler_id The handler identifier returned by @ref register_log_handler for the handler to be unregistered.
	LUNA_RUNTIME_API void unregister_log_handler(usize handler_id);

	//! Enables or disables outputting log messages to platform's default logging device.
	//! @param[in] enabled Specifies `true` to enable logging to standard output. Specify `false` to disable it.
	LUNA_RUNTIME_API void set_log_to_platform_enabled(bool enabled);
	//! Sets the maximum log verbosity level that will be outputted to platform's default logging device.
	//! @param[in] verbosity Specifies the maximum log verbosity level that will be outputted to platform's default logging device.
	LUNA_RUNTIME_API void set_log_to_platform_verbosity(LogVerbosity verbosity);

	//! Enables or disables outputting log messages to the log file.
	//! @param[in] enabled Specifies `true` to enable logging to file. Specify `false` to disable it.
	LUNA_RUNTIME_API void set_log_to_file_enabled(bool enabled);
	//! Sets the file path of the log file.
	//! @param[in] file The file path of the log file. The file path may be absolute or relative to the current working directory.
	//! @remark If the log file path is not set by the user, the default log file path will be `"./Log.txt"`.
	LUNA_RUNTIME_API void set_log_file(const c8* file);
	//! Sets the maximum log verbosity level that will be outputted to the log file.
	//! @param[in] verbosity Specifies the maximum log verbosity level that will be outputted to the log file.
	LUNA_RUNTIME_API void set_log_to_file_verbosity(LogVerbosity verbosity);
	//! Flushes the log-to-file cache and writes all cached logs to the log file.
	//! @remark For performance reasons, when logging-to-file is enabled, log messages will be cached in a log buffer and written
	//! to the log file in one call when the buffer is full. The user can also call @ref flush_log_to_file to flush the cache
	//! manually when needed.
	LUNA_RUNTIME_API void flush_log_to_file();

	//! @}
}