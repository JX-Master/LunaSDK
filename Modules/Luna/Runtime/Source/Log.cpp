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
#include "../StdIO.hpp"
#include "../Event.hpp"

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
		case LogVerbosity::verbose: return "Verbose";
		default: lupanic(); return "";
		}
	}

	struct StdLog
	{
		bool enabled = false;
		LogVerbosity verbosity = LogVerbosity::info;
	};
	static StdLog g_stdlog;
	void std_log(const LogMessage& message)
	{
		StdLog* data = &g_stdlog;
		if (data->enabled && (u8)message.verbosity <= (u8)data->verbosity)
		{
			auto io = get_std_io_stream();
			lutry
			{
				luexp(io->write("[", 1));
				luexp(io->write(message.sender.c_str(), message.sender.size()));
				luexp(io->write("]", 1));
				luexp(io->write(print_verbosity(message.verbosity), 128));
				luexp(io->write(": ", 2));
				luexp(io->write(message.message.c_str(), message.message.size()));
				luexp(io->write("\n", 1));
			}
			lucatch {}
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

	void file_log(const LogMessage& message)
	{
		FileLog* data = g_filelog;
		if (data->enabled && (u8)message.verbosity <= (u8)data->verbosity)
		{
			data->log_buffer.push_back('[');
			data->log_buffer.append(message.sender.c_str());
			data->log_buffer.push_back(']');
			data->log_buffer.append(print_verbosity(message.verbosity));
			data->log_buffer.push_back(':');
			data->log_buffer.push_back(' ');
			data->log_buffer.append(message.message);
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
		register_log_handler(std_log);
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
	LUNA_RUNTIME_API void log(const LogMessage& message)
	{
		MutexGuard guard(g_log_mutex);
		g_log_callbacks(message);
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
	LUNA_RUNTIME_API void log_verbose(const Name& sender, const c8* format, ...)
	{
		VarList args;
		va_start(args, format);
		logv_verbose(sender, format, args);
		va_end(args);
	}
	constexpr usize LOG_STACK_BUFFER_SIZE = 256;
	LUNA_RUNTIME_API void logv_verbose(const Name& sender, const c8* format, VarList args)
	{
		c8 buf[LOG_STACK_BUFFER_SIZE];
		c8* abuf = nullptr;
		i32 len = vsnprintf(buf, LOG_STACK_BUFFER_SIZE, format, args);
		if(len >= LOG_STACK_BUFFER_SIZE)
		{
			abuf = (c8*)memalloc(sizeof(c8) * (len + 1));
			vsnprintf(abuf, len + 1, format, args);
		}
		LogMessage msg;
		msg.sender = sender;
		msg.verbosity = LogVerbosity::info;
		msg.message = abuf ? abuf : buf;
		log(msg);
		if(abuf) memfree(abuf);
	}
	LUNA_RUNTIME_API void log_info(const Name& sender, const c8* format, ...)
	{
		VarList args;
		va_start(args, format);
		logv_info(sender, format, args);
		va_end(args);
	}
	LUNA_RUNTIME_API void logv_info(const Name& sender, const c8* format, VarList args)
	{
		c8 buf[LOG_STACK_BUFFER_SIZE];
		c8* abuf = nullptr;
		i32 len = vsnprintf(buf, LOG_STACK_BUFFER_SIZE, format, args);
		if(len >= LOG_STACK_BUFFER_SIZE)
		{
			abuf = (c8*)memalloc(sizeof(c8) * (len + 1));
			vsnprintf(abuf, len + 1, format, args);
		}
		LogMessage msg;
		msg.sender = sender;
		msg.verbosity = LogVerbosity::info;
		msg.message = abuf ? abuf : buf;
		log(msg);
		if(abuf) memfree(abuf);
	}
	LUNA_RUNTIME_API void log_warning(const Name& sender, const c8* format, ...)
	{
		VarList args;
		va_start(args, format);
		logv_warning(sender, format, args);
		va_end(args);
	}
	LUNA_RUNTIME_API void logv_warning(const Name& sender, const c8* format, VarList args)
	{
		c8 buf[LOG_STACK_BUFFER_SIZE];
		c8* abuf = nullptr;
		i32 len = vsnprintf(buf, LOG_STACK_BUFFER_SIZE, format, args);
		if(len >= LOG_STACK_BUFFER_SIZE)
		{
			abuf = (c8*)memalloc(sizeof(c8) * (len + 1));
			vsnprintf(abuf, len + 1, format, args);
		}
		LogMessage msg;
		msg.sender = sender;
		msg.verbosity = LogVerbosity::warning;
		msg.message = abuf ? abuf : buf;
		log(msg);
		if(abuf) memfree(abuf);
	}
	LUNA_RUNTIME_API void log_error(const Name& sender, const c8* format, ...)
	{
		VarList args;
		va_start(args, format);
		logv_error(sender, format, args);
		va_end(args);
	}
	LUNA_RUNTIME_API void logv_error(const Name& sender, const c8* format, VarList args)
	{
		c8 buf[LOG_STACK_BUFFER_SIZE];
		c8* abuf = nullptr;
		i32 len = vsnprintf(buf, LOG_STACK_BUFFER_SIZE, format, args);
		if(len >= LOG_STACK_BUFFER_SIZE)
		{
			abuf = (c8*)memalloc(sizeof(c8) * (len + 1));
			vsnprintf(abuf, len + 1, format, args);
		}
		LogMessage msg;
		msg.sender = sender;
		msg.verbosity = LogVerbosity::error;
		msg.message = abuf ? abuf : buf;
		log(msg);
		if(abuf) memfree(abuf);
	}
	LUNA_RUNTIME_API void set_log_std_enabled(bool enabled)
	{
		MutexGuard guard(g_log_mutex);
		g_stdlog.enabled = enabled;
	}
	LUNA_RUNTIME_API void set_log_std_verbosity(LogVerbosity verbosity)
	{
		MutexGuard guard(g_log_mutex);
		g_stdlog.verbosity = verbosity;
	}
	LUNA_RUNTIME_API void set_log_file_enabled(bool enabled)
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
	LUNA_RUNTIME_API void set_log_file_verbosity(LogVerbosity verbosity)
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