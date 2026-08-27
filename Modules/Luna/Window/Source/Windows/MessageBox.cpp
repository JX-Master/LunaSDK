/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file MessageBox.cpp
* @author JXMaster
* @date 2026/8/26
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#if defined(LUNA_BUILD_SHARED_LIB)
#define ISOLATION_AWARE_ENABLED 1
#endif
#define LUNA_WINDOW_API LUNA_EXPORT
#include "../../MessageBox.hpp"
#include <Luna/Runtime/String.hpp>
#include <Luna/Runtime/StringUtils.hpp>
#include <Luna/Runtime/TSAssert.hpp>
#include <Luna/Runtime/Unicode.hpp>
#include <Luna/Runtime/Vector.hpp>
#include <Luna/Runtime/Platform/Windows/MiniWin.hpp>
#include <commctrl.h>
#include <objbase.h>

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace Luna
{
    namespace Window
    {
        static bool is_valid_utf8_string(const c8* str)
        {
            const c8* cur = str;
            const c8* end = str + strlen(str);
            while(cur < end)
            {
                usize num_bytes;
                if(failed(utf8_decode_char(cur, (usize)(end - cur), &num_bytes))) return false;
                cur += num_bytes;
            }
            return true;
        }

        static String16 encode_task_dialog_button_text(const c8* text)
        {
            String16 native_text;
            utf8_to_utf16_str(native_text, text);
            usize num_ampersands = 0;
            for(c16 ch : native_text)
            {
                if(ch == u'&') ++num_ampersands;
            }
            if(!num_ampersands) return native_text;
            String16 escaped_text;
            escaped_text.reserve(native_text.size() + num_ampersands);
            for(c16 ch : native_text)
            {
                if(ch == u'&') escaped_text.push_back(u'&');
                escaped_text.push_back(ch);
            }
            return escaped_text;
        }

        struct ScopedCOMApartment
        {
            HRESULT result;

            ScopedCOMApartment() :
                result(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)) {}

            ~ScopedCOMApartment()
            {
                if(SUCCEEDED(result)) CoUninitialize();
            }
        };

        LUNA_WINDOW_API R<usize> message_box(const c8* text, const c8* title, Span<const c8*> buttons,
            MessageBoxIcon icon, usize default_button_index, usize cancel_button_index)
        {
            lutsassert_main_thread();
            if(!text || !title || !is_valid_utf8_string(text) || !is_valid_utf8_string(title) || buttons.empty() ||
                default_button_index >= buttons.size() || (cancel_button_index != USIZE_MAX &&
                (cancel_button_index >= buttons.size() || cancel_button_index == default_button_index)))
            {
                return E_BAD_ARGUMENTS;
            }
            constexpr int FIRST_BUTTON_ID = 1000;
            if(buttons.size() > (usize)(I32_MAX - FIRST_BUTTON_ID)) return E_DATA_TOO_BIG;
            for(const c8* button_text : buttons)
            {
                if(!button_text || !button_text[0] || !is_valid_utf8_string(button_text)) return E_BAD_ARGUMENTS;
            }

            PCWSTR native_icon = nullptr;
            switch(icon)
            {
                case MessageBoxIcon::none:
                case MessageBoxIcon::question:
                    break;
                case MessageBoxIcon::information:
                    native_icon = TD_INFORMATION_ICON;
                    break;
                case MessageBoxIcon::warning:
                    native_icon = TD_WARNING_ICON;
                    break;
                case MessageBoxIcon::error:
                    native_icon = TD_ERROR_ICON;
                    break;
                default:
                    return E_BAD_ARGUMENTS;
            }

            ScopedCOMApartment com;
            if(FAILED(com.result))
            {
                if(com.result == RPC_E_CHANGED_MODE)
                {
                    return set_error(E_BAD_CALLING_TIME,
                        "The calling thread uses an incompatible COM apartment; TaskDialogIndirect requires STA.");
                }
                if(com.result == E_OUTOFMEMORY) return E_OUT_OF_MEMORY;
                return set_error(E_BAD_PLATFORM_CALL, "CoInitializeEx failed with HRESULT 0x%08X.", (u32)com.result);
            }

            String16 native_text;
            String16 native_title;
            utf8_to_utf16_str(native_text, text);
            utf8_to_utf16_str(native_title, title);
            Vector<String16> native_button_texts;
            native_button_texts.reserve(buttons.size());
            for(const c8* button_text : buttons)
            {
                native_button_texts.push_back(encode_task_dialog_button_text(button_text));
            }
            Vector<TASKDIALOG_BUTTON> native_buttons;
            native_buttons.reserve(buttons.size());
            for(usize i = 0; i < buttons.size(); ++i)
            {
                TASKDIALOG_BUTTON button;
                button.nButtonID = i == cancel_button_index ? IDCANCEL : FIRST_BUTTON_ID + (int)i;
                button.pszButtonText = (PCWSTR)native_button_texts[i].c_str();
                native_buttons.push_back(button);
            }

            TASKDIALOGCONFIG config = {};
            config.cbSize = sizeof(config);
            config.hInstance = GetModuleHandleW(nullptr);
            if(cancel_button_index != USIZE_MAX) config.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION;
            config.pszWindowTitle = (PCWSTR)native_title.c_str();
            config.pszContent = (PCWSTR)native_text.c_str();
            config.pszMainIcon = native_icon;
            config.cButtons = (UINT)native_buttons.size();
            config.pButtons = native_buttons.data();
            config.nDefaultButton = native_buttons[default_button_index].nButtonID;

            int selected_button_id = 0;
            HRESULT result = TaskDialogIndirect(&config, &selected_button_id, nullptr, nullptr);
            if(FAILED(result))
            {
                if(result == E_OUTOFMEMORY) return E_OUT_OF_MEMORY;
                return set_error(E_BAD_PLATFORM_CALL, "TaskDialogIndirect failed with HRESULT 0x%08X.", (u32)result);
            }
            if(selected_button_id == IDCANCEL)
            {
                if(cancel_button_index != USIZE_MAX) return cancel_button_index;
                return E_INTERRUPTED;
            }
            if(selected_button_id >= FIRST_BUTTON_ID)
            {
                usize button_index = (usize)(selected_button_id - FIRST_BUTTON_ID);
                if(button_index < buttons.size() && button_index != cancel_button_index) return button_index;
            }
            return set_error(E_BAD_PLATFORM_CALL, "TaskDialogIndirect returned unexpected button ID %d.",
                selected_button_id);
        }
    }
}
