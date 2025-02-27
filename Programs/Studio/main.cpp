/*!
* This file is a portion of Luna SDK.
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

    void run_editor()
    {
        set_log_to_platform_enabled(true);
        set_log_to_platform_verbosity(LogVerbosity::error);
        lupanic_if_failed(add_modules({module_variant_utils(),
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
        auto r = init_modules();
        if (failed(r))
        {
            log_error("App", explain(r.errcode()));
            return;
        }
        if (failed(init_env())) return;

        // Run project selector.
        auto project = select_project();
        if (failed(project))
        {
            memdelete(g_env);
            g_env = nullptr;
            return;
        }

        // Run main editor.
        run_main_editor(project.get());

        memdelete(g_env);
        g_env = nullptr;

        return;
    }

    void set_current_dir_to_process_path()
    {
        Path p = get_process_path();
        p.pop_back();
        luassert_always(succeeded(set_current_dir(p.encode().c_str())));
    }
}

using namespace Luna;

int main(){
    luassert_always(Luna::init());
    set_current_dir_to_process_path();
    run_editor();
    Luna::close();
    return 0;
}