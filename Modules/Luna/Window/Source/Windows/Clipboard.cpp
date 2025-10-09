/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Clipboard.cpp
* @author JXMaster
* @date 2025/10/9
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_WINDOW_API LUNA_EXPORT
#include "../../Clipboard.hpp"
#include <Luna/Runtime/Unicode.hpp>
#include <Luna/Runtime/Platform/Windows/MiniWin.hpp>

namespace Luna
{
    namespace Window
    {
        LUNA_WINDOW_API RV get_clipboard_text(String& out_text)
        {
            if (!OpenClipboard(NULL))
            {
                DWORD error = GetLastError();
                return set_error(BasicError::bad_platform_call(), "OpenClipboard failed: %d", error);
            }
            
            HANDLE hData = GetClipboardData(CF_UNICODETEXT);
            if (hData == NULL)
            {
                DWORD error = GetLastError();
                CloseClipboard();
                return set_error(BasicError::bad_platform_call(), "GetClipboardData failed: %d", error);
            }
            
            wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hData));
            if (pszText == NULL)
            {
                DWORD error = GetLastError();
                CloseClipboard();
                return set_error(BasicError::bad_platform_call(), "GlobalLock failed: %d", error);
            }
            
            // Convert UTF-16 to UTF-8
            usize utf8_size = utf16_to_utf8_len((const char16_t*)pszText);
            StackAllocator alloc;
            c8* buf = (c8*)alloc.allocate(utf8_size + 1);
            utf16_to_utf8(buf, utf8_size + 1, (const char16_t*)pszText);
            out_text.append(buf);
            GlobalUnlock(hData);
            CloseClipboard();
            
            return ok;
        }
        
        LUNA_WINDOW_API RV set_clipboard_text(const c8* text, usize size)
        {
            lucheck(text);
            
            // Calculate actual size
            usize actual_size = size;
            // Find null terminator within size limit
            for (usize i = 0; i < size; ++i)
            {
                if (text[i] == '\0')
                {
                    actual_size = i;
                    break;
                }
            }
            
            // Convert UTF-8 to UTF-16
            usize utf16_size = utf8_to_utf16_len(text, actual_size);
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (utf16_size + 1) * sizeof(wchar_t));
            if (!hMem)
            {
                DWORD error = GetLastError();
                return set_error(BasicError::out_of_memory(), "GlobalAlloc failed: %d", error);
            }
            
            wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hMem));
            if (!pszText)
            {
                DWORD error = GetLastError();
                GlobalFree(hMem);
                return set_error(BasicError::bad_platform_call(), "GlobalLock failed: %d", error);
            }
            
            utf8_to_utf16((char16_t*)pszText, utf16_size + 1, text, actual_size);
            GlobalUnlock(hMem);
            
            if (!OpenClipboard(NULL))
            {
                DWORD error = GetLastError();
                GlobalFree(hMem);
                return set_error(BasicError::bad_platform_call(), "OpenClipboard failed: %d", error);
            }
            
            EmptyClipboard();
            
            if (!SetClipboardData(CF_UNICODETEXT, hMem))
            {
                DWORD error = GetLastError();
                CloseClipboard();
                GlobalFree(hMem);
                return set_error(BasicError::bad_platform_call(), "SetClipboardData failed: %d", error);
            }
            
            CloseClipboard();
            return ok;
        }
    }
}

