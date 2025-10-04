/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ProjectSelector.hpp
* @author JXMaster
* @date 2020/4/20
*/
#pragma once
#include <Luna/Runtime/Path.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Window/Window.hpp>
#include <Luna/RHI/SwapChain.hpp>
#include <Luna/RHI/CommandBuffer.hpp>

namespace Luna
{
    struct RecentFileRecord
    {
        u64 m_last_use_time;
        Path m_path;
    };

    struct ProjectSelector
    {
        Ref<Window::IWindow> window;
        Ref<RHI::ISwapChain> swap_chain;
        Ref<RHI::ICommandBuffer> cmdbuf;

        String new_solution_name;
        Vector<RecentFileRecord> recents;
        Path selected_path;
        bool create_dir = true;
        bool exiting = false;

        RV init();
        RV update();
        Path get_selected_path();
    };

    extern ProjectSelector* g_project_selector;
}