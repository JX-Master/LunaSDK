/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Log.cpp
* @author JXMaster
* @date 2023/9/7
*/
#include "../../OS.hpp"
#include <unistd.h>

namespace Luna
{
	namespace OS
	{
		inline const char* print_verbosity(LogVerbosity verbosity, usize& len)
		{
			switch (verbosity)
			{
			case LogVerbosity::fatal_error: 
				len = 13;
				return "Fatal Error: ";
			case LogVerbosity::error: 
				len = 7;
				return "Error: ";
			case LogVerbosity::warning: 
				len = 9;
				return "Warning: ";
			case LogVerbosity::info: 
				len = 6;
				return "Info: ";
			case LogVerbosity::debug:
				len = 7;
				return "Debug: ";
			case LogVerbosity::verbose:
				len = 9;
				return "Verbose: ";
			default: lupanic(); 
				len = 0;
				return "";
			}
		}
		void log(LogVerbosity verbosity, const c8* tag, usize tag_len, const c8* message, usize message_len)
		{
			write(STDOUT_FILENO, "[", 1);
			write(STDOUT_FILENO, tag, tag_len);
			write(STDOUT_FILENO, "]", 1);
			usize verbosity_len;
			const char* verbosity_str = print_verbosity(verbosity, verbosity_len);
			write(STDOUT_FILENO, verbosity_str, verbosity_len);
			write(STDOUT_FILENO, message, message_len);
			write(STDOUT_FILENO, "\n", 1);
		}
	}
}