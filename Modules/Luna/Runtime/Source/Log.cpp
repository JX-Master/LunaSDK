/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Log.cpp
* @author JXMaster
* @date 2022/6/30
*/
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "../Log.hpp"
#include "../Mutex.hpp"
#include "../File.hpp"
#include "../Event.hpp"
#include "OS.hpp"

namespace Luna
{	
	static Event<log_callback_t> g_log_callbacks;

	static Ref<IMutex> g_log_mutex;

	inline const c8* print_verbosity(LogVerbosity verbosity)
	{
		switch (verbosity)
		{
		case LogVerbosity::fatal_error: return "Fatal Error";
		case LogVerbosity::error: return "Error";
		case LogVerbosity::warning: return "Warning";
		case LogVerbosity::info: return "Info";
		case LogVerbosity::debug: return "Debug";
		case LogVerbosity::verbose: return "Verbose";
		default: lupanic(); return "";
		}
	}

	struct PlatformLog
	{
		bool enabled = false;
		LogVerbosity verbosity = LogVerbosity::info;
	};
	static PlatformLog g_platform_log;
	void platform_log(LogVerbosity verbosity, const c8* tag, usize tag_length, const c8* message, usize message_length)
	{
		if (g_platform_log.enabled && (u8)verbosity <= (u8)g_platform_log.verbosity)
		{
			OS::log(verbosity, tag, tag_length, message, message_length);
		}
	}

	struct FileLog
	{
		bool enabled;
		LogVerbosity verbosity = LogVerbosity::verbose;
		Name filename;
		String log_buffer;
	};

	static FileLog* g_filelog;

	static void flush_log_file()
	{
		if (!g_filelog->log_buffer.empty())
		{
			lutry
			{
				lulet(f, open_file(g_filelog->filename.c_str(), FileOpenFlag::write, FileCreationMode::open_always));
				luexp(f->seek(0, SeekMode::end));
				luexp(f->write(g_filelog->log_buffer.data(), g_filelog->log_buffer.size()));
				g_filelog->log_buffer.clear();
			}
			lucatch
			{
				return;
			}
		}
	}

	void file_log(LogVerbosity verbosity, const c8* tag, usize tag_length, const c8* message, usize message_length)
	{
		FileLog* data = g_filelog;
		if (data->enabled && (u8)verbosity <= (u8)data->verbosity)
		{
			data->log_buffer.push_back('[');
			data->log_buffer.append(tag, tag_length);
			data->log_buffer.push_back(']');
			data->log_buffer.append(print_verbosity(verbosity));
			data->log_buffer.push_back(':');
			data->log_buffer.push_back(' ');
			data->log_buffer.append(message, message_length);
			data->log_buffer.push_back('\n');
			if (data->log_buffer.size() > 64_kb)
			{
				flush_log_file();
			}
		}
	}

	void log_init()
	{
		g_log_mutex = new_mutex();
		register_log_handler(platform_log);
		g_filelog = memnew<FileLog>();
		g_filelog->filename = "./Log.txt";
		register_log_handler(file_log);
	}
	void log_close()
	{
		flush_log_file();
		memdelete(g_filelog);
		g_log_callbacks.clear();
		g_log_mutex = nullptr;
	}
	LUNA_RUNTIME_API void log(LogVerbosity verbosity, const c8* tag, const c8* format, ...)
	{
		VarList args;
		va_start(args, format);
		logv(verbosity, tag, format, args);
		va_end(args);
	}
	constexpr usize LOG_STACK_BUFFER_SIZE = 256;
	LUNA_RUNTIME_API void logv(LogVerbosity verbosity, const c8* tag, const c8* format, VarList args)
	{
		c8 buf[LOG_STACK_BUFFER_SIZE];
		c8* abuf = nullptr;
		i32 len = vsnprintf(buf, LOG_STACK_BUFFER_SIZE, format, args);
		if (len >= LOG_STACK_BUFFER_SIZE)
		{
			abuf = (c8*)memalloc(sizeof(c8) * (len + 1));
			len = vsnprintf(abuf, len + 1, format, args);
		}
		c8* use_buf = abuf ? abuf : buf;
		MutexGuard guard(g_log_mutex);
		if(!tag) tag = "";
		g_log_callbacks(verbosity, tag, strlen(tag), use_buf, len);
		guard.unlock();
		if (abuf) memfree(abuf);
	}
	LUNA_RUNTIME_API usize register_log_handler(const Function<log_callback_t>& handler)
	{
		MutexGuard guard(g_log_mutex);
		return g_log_callbacks.add_handler(handler);
	}
	LUNA_RUNTIME_API void unregister_log_handler(usize handler_id)
	{
		MutexGuard guard(g_log_mutex);
		g_log_callbacks.remove_handler(handler_id);
	}
	LUNA_RUNTIME_API void log_verbose(const c8* tag, const c8* format, ...)
	{
		VarList args;
		va_start(args, format);
		logv_verbose(tag, format, args);
		va_end(args);
	}
	LUNA_RUNTIME_API void logv_verbose(const c8* tag, const c8* format, VarList args)
	{
		logv(LogVerbosity::verbose, tag, format, args);
	}
	LUNA_RUNTIME_API void log_debug(const c8* tag, const c8* format, ...)
	{
		VarList args;
		va_start(args, format);
		logv_debug(tag, format, args);
		va_end(args);
	}
	LUNA_RUNTIME_API void logv_debug(const c8* tag, const c8* format, VarList args)
	{
		logv(LogVerbosity::debug, tag, format, args);
	}
	LUNA_RUNTIME_API void log_info(const c8* tag, const c8* format, ...)
	{
		VarList args;
		va_start(args, format);
		logv_info(tag, format, args);
		va_end(args);
	}
	LUNA_RUNTIME_API void logv_info(const c8* tag, const c8* format, VarList args)
	{
		logv(LogVerbosity::info, tag, format, args);
	}
	LUNA_RUNTIME_API void log_warning(const c8* tag, const c8* format, ...)
	{
		VarList args;
		va_start(args, format);
		logv_warning(tag, format, args);
		va_end(args);
	}
	LUNA_RUNTIME_API void logv_warning(const c8* tag, const c8* format, VarList args)
	{
		logv(LogVerbosity::warning, tag, format, args);
	}
	LUNA_RUNTIME_API void log_error(const c8* tag, const c8* format, ...)
	{
		VarList args;
		va_start(args, format);
		logv_error(tag, format, args);
		va_end(args);
	}
	LUNA_RUNTIME_API void logv_error(const c8* tag, const c8* format, VarList args)
	{
		logv(LogVerbosity::error, tag, format, args);
	}
	LUNA_RUNTIME_API void set_log_to_platform_enabled(bool enabled)
	{
		MutexGuard guard(g_log_mutex);
		g_platform_log.enabled = enabled;
	}
	LUNA_RUNTIME_API void set_log_to_platform_verbosity(LogVerbosity verbosity)
	{
		MutexGuard guard(g_log_mutex);
		g_platform_log.verbosity = verbosity;
	}
	LUNA_RUNTIME_API void set_log_to_file_enabled(bool enabled)
	{
		MutexGuard guard(g_log_mutex);
		g_filelog->enabled = enabled;
	}
	LUNA_RUNTIME_API void set_log_file(const c8* file)
	{
		MutexGuard guard(g_log_mutex);
		flush_log_file();
		g_filelog->filename = file;
	}
	LUNA_RUNTIME_API void set_log_to_file_verbosity(LogVerbosity verbosity)
	{
		MutexGuard guard(g_log_mutex);
		g_filelog->verbosity = verbosity;
	}
	LUNA_RUNTIME_API void flush_log_to_file()
	{
		MutexGuard guard(g_log_mutex);
		flush_log_file();
	}
}