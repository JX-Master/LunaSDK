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
	
	//! @brief Defines all log verbosity levels.
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
		//! Verbose message that should not be displayed in normal condition.
		verbose = 4,
	};

	//! @brief Represents one log message.
	struct LogMessage
	{
		//! The function/module that sends the log.
		Name sender;
		//! The log message.
		String message;
		//! The verbosity of the log.
		LogVerbosity verbosity;
		//! Extra arguments attached to the log.
		Variant extra;
	};

	using log_callback_t = void(const LogMessage& message);

	//! @brief Outputs one log to the log system.
	//! @param[in] message The log message to output.
	LUNA_RUNTIME_API void log(const LogMessage& message);
	
	//! @brief Outputs one log message with @ref LogVerbosity::verbose verbosity.
	//! @param[in] sender The function/module that sends the log.
	//! @param[in] format The formatting syntax used to format the log message.
	//! @param[in] args Arguments used to format the log message.
	LUNA_RUNTIME_API void log_verbose(const Name& sender, const c8* format, ...);
	//! @brief Outputs one log message with @ref LogVerbosity::verbose verbosity.
	//! @param[in] sender The function/module that sends the log.
	//! @param[in] format The formatting syntax used to format the log message.
	//! @param[in] args Arguments used to format the log message.
	LUNA_RUNTIME_API void logv_verbose(const Name& sender, const c8* format, VarList args);
	//! @brief Outputs one log message with @ref LogVerbosity::info verbosity.
	//! @param[in] sender The function/module that sends the log.
	//! @param[in] format The formatting syntax used to format the log message.
	//! @param[in] args Arguments used to format the log message.
	LUNA_RUNTIME_API void log_info(const Name& sender, const c8* format, ...);
	//! @brief Outputs one log message with @ref LogVerbosity::info verbosity.
	//! @param[in] sender The function/module that sends the log.
	//! @param[in] format The formatting syntax used to format the log message.
	//! @param[in] args Arguments used to format the log message.
	LUNA_RUNTIME_API void logv_info(const Name& sender, const c8* format, VarList args);
	//! @brief Outputs one log message with @ref LogVerbosity::warning verbosity.
	//! @param[in] sender The function/module that sends the log.
	//! @param[in] format The formatting syntax used to format the log message.
	//! @param[in] args Arguments used to format the log message.
	LUNA_RUNTIME_API void log_warning(const Name& sender, const c8* format, ...);
	//! @brief Outputs one log message with @ref LogVerbosity::warning verbosity.
	//! @param[in] sender The function/module that sends the log.
	//! @param[in] format The formatting syntax used to format the log message.
	//! @param[in] args Arguments used to format the log message.
	LUNA_RUNTIME_API void logv_warning(const Name& sender, const c8* format, VarList args);
	//! @brief Outputs one log message with @ref LogVerbosity::error verbosity.
	//! @param[in] sender The function/module that sends the log.
	//! @param[in] format The formatting syntax used to format the log message.
	//! @param[in] args Arguments used to format the log message.
	LUNA_RUNTIME_API void log_error(const Name& sender, const c8* format, ...);
	//! @brief Outputs one log message with @ref LogVerbosity::error verbosity.
	//! @param[in] sender The function/module that sends the log.
	//! @param[in] format The formatting syntax used to format the log message.
	//! @param[in] args Arguments used to format the log message.
	LUNA_RUNTIME_API void logv_error(const Name& sender, const c8* format, VarList args);

	//! @brief Registers one custom log handler that will be called when a new log message is spawned.
	//! @param[in] handler The handler to register.
	//! @return Returns one handler identifier that can be used to register the handler.
	LUNA_RUNTIME_API usize register_log_handler(const Function<log_callback_t>& handler);
	//! @brief Unregisters one registered log handler.
	//! @param[in] handler_id The handler identifier returned by @ref register_log_handler for the handler to be unregistered.
	LUNA_RUNTIME_API void unregister_log_handler(usize handler_id);

	//! @brief Enables or disables outputting log messages to standard output.
	//! @param[in] enabled Specifies `true` to enable logging to standard output. Specify `false` to disable it.
	LUNA_RUNTIME_API void set_log_std_enabled(bool enabled);
	//! @brief Sets the maximum log verbosity level that will be outputted to standard output.
	//! @param[in] verbosity Specifies the maximum log verbosity level that will be outputted to standard output.
	LUNA_RUNTIME_API void set_log_std_verbosity(LogVerbosity verbosity);

	//! @brief Enables or disables outputting log messages to the log file.
	//! @param[in] enabled Specifies `true` to enable logging to file. Specify `false` to disable it.
	LUNA_RUNTIME_API void set_log_file_enabled(bool enabled);
	//! @brief Sets the file path of the log file.
	//! @param[in] file The file path of the log file. The file path may be absolute or relative to the current working directory.
	//! @remark If the log file path is not set by the user, the default log file path will be `"./Log.txt"`.
	LUNA_RUNTIME_API void set_log_file(const c8* file);
	//! @brief Sets the maximum log verbosity level that will be outputted to the log file.
	//! @param[in] verbosity Specifies the maximum log verbosity level that will be outputted to the log file.
	LUNA_RUNTIME_API void set_log_file_verbosity(LogVerbosity verbosity);
	//! @brief Flushes the log-to-file cache and writes all cached logs to the log file.
	//! @remark For performance reasons, when logging-to-file is enabled, log messages will be cached in a log buffer and written
	//! to the log file in one call when the buffer is full. The user can also call @ref flush_log_to_file to flush the cache
	//! manually when needed.
	LUNA_RUNTIME_API void flush_log_to_file();

	//! @}
}