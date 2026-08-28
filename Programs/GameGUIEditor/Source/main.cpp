/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file main.cpp
* @author JXMaster
* @date 2026/8/26
*/
#include "EditorApp.hpp"
#include <Luna/Font/Font.hpp>
#include <Luna/RHIUtility/RHIUtility.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/VG/VG.hpp>
#include <Luna/Window/Application.hpp>
#include <Luna/Window/AppMain.hpp>
#include <cstdio>
#include <cstring>

using namespace Luna;

int luna_main(int argc, const char* argv[])
{
    i32 max_frames = -1;
    bool discard_smoke = false;
    const c8* workspace_path = nullptr;
    for(int i = 1; i < argc; ++i)
    {
        i32 parsed = 0;
        if(sscanf(argv[i], "--frames=%d", &parsed) == 1) max_frames = parsed;
        else if(!strcmp(argv[i], "--discard-smoke")) discard_smoke = true;
        else if(!strncmp(argv[i], "--workspace=", 12)) workspace_path = argv[i] + 12;
    }
    lupanic_if_failed(Luna::init());
    lupanic_if_failed(add_modules({
        module_window(),
        module_rhi(),
        module_rhi_utility(),
        module_font(),
        module_vg(),
        GUI::module_gui(),
        EditorGUI::module_editor_gui(),
        GUIWindow::module_gui_window(),
        GameGUI::module_game_gui(),
        Frontend::module_frontend()
    }));
    Window::StartupParams startup_params;
    startup_params.name = GameGUIEditor::Internal::APP_NAME;
    Window::set_startup_params(startup_params);
    lupanic_if_failed(init_modules());
    {
        GameGUIEditor::Internal::EditorApp app;
        app.max_frames = max_frames;
        app.discard_smoke = discard_smoke;
        if(workspace_path) app.workspace_path = workspace_path;
        lupanic_if_failed(app.init());
        lupanic_if_failed(app.run());
    }
    Luna::close();
    return 0;
}
