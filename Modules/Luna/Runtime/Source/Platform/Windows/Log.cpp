/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Log.cpp
* @author JXMaster
* @date 2023/9/7
*/
#include "../Log.hpp"
#include "../../../Platform/Windows/MiniWin.hpp"
#include "Utils.hpp"

namespace Luna
{
    namespace Platform
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

            usize wlen_tag, wlen_message;
            wchar_t* wtag = utf8_to_wchar_buffered(tag, tag_len, &wlen_tag);
            wchar_t* wmessage = utf8_to_wchar_buffered(message, message_len, &wlen_message);
            WriteConsoleW(hConsole, L"[", 1, NULL, NULL);
            WriteConsoleW(hConsole, wtag, wlen_tag, NULL, NULL);
            WriteConsoleW(hConsole, L"]", 1, NULL, NULL);
            usize verbosity_len;
            const wchar_t* wverbosity = print_verbosity(verbosity, verbosity_len);
            WriteConsoleW(hConsole, wverbosity, verbosity_len, NULL, NULL);
            WriteConsoleW(hConsole, wmessage, wlen_message, NULL, NULL);
            WriteConsoleW(hConsole, L"\n", 1, NULL, NULL);
            memfree(wtag);
            memfree(wmessage);
        }
    }
}