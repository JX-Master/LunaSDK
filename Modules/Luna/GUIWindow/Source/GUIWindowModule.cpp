/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUIWindowModule.cpp
* @author JXMaster
* @date 2026/6/18
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_WINDOW_API LUNA_EXPORT
#include "../GUIWindow.hpp"
#include <Luna/HID/HID.hpp>

namespace Luna
{
    namespace GUIWindow
    {
        struct GUIWindowModule : public Module
        {
            virtual const c8* get_name() override { return "GUIWindow"; }
            virtual RV on_register() override
            {
                return add_dependency_modules(this, {
                    GUICore::module_gui_core(),
                    module_window(),
                    module_hid()});
            }
            virtual RV on_init() override { return ok; }
            virtual void on_close() override {}
        };

        LUNA_GUI_WINDOW_API Module* module_gui_window()
        {
            static GUIWindowModule m;
            return &m;
        }
    }
}
