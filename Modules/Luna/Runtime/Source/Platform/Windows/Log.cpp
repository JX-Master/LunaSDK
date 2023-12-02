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
#include "../../../Platform/Windows/MiniWin.hpp"
#include "../../../Unicode.hpp"

namespace Luna
{
	namespace OS
	{
		inline const wchar_t* print_verbosity(LogVerbosity verbosity, usize& len)
		{
			switch (verbosity)
			{
			case LogVerbosity::fatal_error: 
				len = 13;
				return L"Fatal Error: ";
			case LogVerbosity::error: 
				len = 7;
				return L"Error: ";
			case LogVerbosity::warning: 
				len = 9;
				return L"Warning: ";
			case LogVerbosity::info: 
				len = 6;
				return L"Info: ";
			case LogVerbosity::debug:
				len = 7;
				return L"Debug: ";
			case LogVerbosity::verbose:
				len = 9;
				return L"Verbose: ";
			default: lupanic(); 
				len = 0;
				return L"";
			}
		}
		void log(LogVerbosity verbosity, const c8* tag, usize tag_len, const c8* message, usize message_len)
		{
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			switch (verbosity)
			{
			case LogVerbosity::error:
			case LogVerbosity::fatal_error:
				SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
				break;
			case LogVerbosity::warning:
				SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
				break;
			default:
				SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
				break;
			}
			// Encode the text in UTF-16.
			usize wlen_tag = utf8_to_utf16_len(tag, tag_len);
			wchar_t* wtag = (wchar_t*)alloca(sizeof(wchar_t) * (wlen_tag + 1));
			wlen_tag = utf8_to_utf16((c16*)wtag, wlen_tag + 1, tag, tag_len);
			usize wlen = utf8_to_utf16_len(message, message_len);
			wchar_t* wmessage = (wchar_t*)alloca(sizeof(wchar_t) * (wlen + 1));
			wlen = utf8_to_utf16((c16*)wmessage, wlen + 1, message, message_len);
			WriteConsoleW(hConsole, L"[", 1, NULL, NULL);
			WriteConsoleW(hConsole, wtag, wlen_tag, NULL, NULL);
			WriteConsoleW(hConsole, L"]", 1, NULL, NULL);
			usize verbosity_len;
			const wchar_t* wverbosity = print_verbosity(verbosity, verbosity_len);
			WriteConsoleW(hConsole, wverbosity, verbosity_len, NULL, NULL);
			WriteConsoleW(hConsole, wmessage, wlen, NULL, NULL);
			WriteConsoleW(hConsole, L"\n", 1, NULL, NULL);
		}
	}
}