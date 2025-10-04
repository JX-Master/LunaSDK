/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file main.cpp
* @author JXMaster
* @date 2020/4/20
*/
#include "StudioHeader.hpp"
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/File.hpp>
#include "ProjectSelector.hpp"
#include "MainEditor.hpp"
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/VariantUtils/VariantUtils.hpp>
#include <Luna/RG/RG.hpp>
#include <Luna/JobSystem/JobSystem.hpp>

#include <Luna/Window/AppMain.hpp>

namespace Luna
{
    AppEnv* g_env = nullptr;

    RV init_env()
    {
        lutry
        {
            g_env = memnew<AppEnv>();
            g_env->device = RHI::get_main_device();
            u32 num_queues = g_env->device->get_num_command_queues();
            g_env->graphics_queue = U32_MAX;
            g_env->async_compute_queue = U32_MAX;
            g_env->async_copy_queue = U32_MAX;
            for (u32 i = 0; i < num_queues; ++i)
            {
                auto desc = g_env->device->get_command_queue_desc(i);
                if (desc.type == RHI::CommandQueueType::graphics && g_env->graphics_queue == U32_MAX)
                {
                    g_env->graphics_queue = i;
                }
                if (desc.type == RHI::CommandQueueType::compute && g_env->async_compute_queue == U32_MAX)
                {
                    g_env->async_compute_queue = i;
                }
                if (desc.type == RHI::CommandQueueType::copy && g_env->async_copy_queue == U32_MAX)
                {
                    g_env->async_copy_queue = i;
                }
            }
            if (g_env->async_compute_queue == U32_MAX) g_env->async_compute_queue = g_env->graphics_queue;
            if(g_env->async_copy_queue == U32_MAX) g_env->async_copy_queue = g_env->graphics_queue;
        }
        lucatchret;
        return ok;
    }

    void set_current_dir_to_process_path()
    {
        Path p = get_process_path();
        p.pop_back();
        luassert_always(succeeded(set_current_dir(p.encode().c_str())));
    }
}

namespace Luna
{
    Window::AppStatus app_init(opaque_t* app_state, int argc, char* argv[])
    {
        bool r = Luna::init();
        if(!r) return Window::AppStatus::failing;
        set_current_dir_to_process_path();
        set_log_to_platform_enabled(true);
        set_log_to_platform_verbosity(LogVerbosity::error);
        lutry
        {
            luexp(add_modules({module_variant_utils(),
                module_hid(),
                module_window(),
                module_rhi(),
                module_image(),
                module_font(),
                module_imgui(),
                module_asset(),
                module_obj_loader(),
                module_rg(),
                module_job_system(),
                module_shader_compiler()}));
            luexp(init_modules());
            luexp(init_env());
            g_project_selector = memnew<ProjectSelector>();
            luexp(g_project_selector->init());
        }
        lucatch
        {
            log_error("App", explain(luerr));
            return Window::AppStatus::failing;
        }
        return Window::AppStatus::running;
    }

    Window::AppStatus app_update(opaque_t app_state)
    {
        lutry
        {
            if(g_project_selector)
            {
                luexp(g_project_selector->update());
                if(g_project_selector->exiting)
                {
                    Path path = g_project_selector->selected_path;
                    if(path.empty())
                    {
                        return Window::AppStatus::exiting;
                    }
                    // Switch to main editor.
                    memdelete(g_project_selector);
                    g_project_selector = nullptr;
                    g_main_editor = memnew<MainEditor>();
                    luexp(g_main_editor->init(path));
                }
            }
            else
            {
                luassert(g_main_editor);
                luexp(g_main_editor->update());
                if(g_main_editor->m_exiting)
                {
                    g_main_editor->close();
                    return Window::AppStatus::exiting;
                }
            }
        }
        lucatch
        {
            log_error("App", explain(luerr));
            return Window::AppStatus::failing;
        }
        return Window::AppStatus::running;
    }
    
    void app_close(opaque_t app_state, Window::AppStatus status)
    {
        Asset::close();
        if(g_main_editor)
        {
            memdelete(g_main_editor);
            g_main_editor = nullptr;
        }
        if(g_project_selector)
        {
            memdelete(g_project_selector);
            g_project_selector = nullptr;
        }
        if(g_env)
        {
            memdelete(g_env);
            g_env = nullptr;
        }
        Luna::close();
    }
}