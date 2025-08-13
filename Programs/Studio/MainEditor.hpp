/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MainEditor.hpp
* @author JXMaster
* @date 2020/4/27
*/
#pragma once
#include "StudioHeader.hpp"
#include "AssetBrowser.hpp"
#include "MemoryProfiler.hpp"
#include <Luna/Runtime/HashMap.hpp>
#include "Operation.hpp"
#include <Luna/Runtime/RingDeque.hpp>

namespace Luna
{
    constexpr c8 APP_NAME[] = "Luna Studio";

    struct MainEditor
    {
        lustruct("MainEditor", "{CF004929-E981-4E1D-A4AE-96EEC79AD1EB}");

        Path m_project_path;

        Ref<Window::IWindow> m_window;
        Ref<RHI::ISwapChain> m_swap_chain;
        Ref<RHI::ICommandBuffer> m_cmdbuf;

        Ref<AssetBrowser> m_asset_browsers[4];
        bool m_asset_browsers_enabled[4] = { true, false, false, false };

        Vector<Ref<IAssetEditor>> m_editors;

        MemoryProfiler m_memory_profiler;
        usize m_memory_profiler_callback_handle;
        bool m_memory_profiler_window_enabled = false;

        //u32 m_next_asset_browser_index;

        bool m_exiting;

        u32 m_main_window_width;
        u32 m_main_window_height;

    private:
        struct AssetVersion
        {
            u32 edit_version = 1;
            u32 save_version = 0;
        };

        // Edited and unsaved assets.
        // Do not access this map directly, use `mark_asset_as_edited`, `mark_asset_as_saved`,
        // `has_any_unsaved_changes`, `has_unsaved_changes`, `get_asset_edit_version` instead.
        HashMap<Asset::asset_t, AssetVersion> m_assets_version;

        void draw_main_menu_bar();

    public:

        // Undo & redo stack.
        RingDeque<Ref<Operation>> m_operations_stack;
        // The last executed operation.
        usize m_operations_stack_top = 0;

        MainEditor() :
            m_exiting(false),
            m_main_window_width(0),
            m_main_window_height(0) {}
        //m_next_asset_browser_index(0) {}

        RV init(const Path& project_path);

        RV update();

        void close();

        RV save_all();

        bool can_undo()
        {
            return m_operations_stack_top > 0;
        }
        bool can_redo()
        {
            return m_operations_stack_top < m_operations_stack.size();
        }
        void execute(Operation* op);
        virtual void undo();
        virtual void redo();

        void mark_asset_as_edited(Asset::asset_t asset)
        {
            auto iter = m_assets_version.find(asset);
            if(iter == m_assets_version.end())
            {
                m_assets_version.insert(make_pair(asset, AssetVersion()));
            }
            else
            {
                (iter->second.edit_version)++;
            }
        }
        void clear_asset_edited_flag(Asset::asset_t asset)
        {
            m_assets_version.erase(asset);
        }
        void mark_asset_as_saved(Asset::asset_t asset)
        {
            auto iter = m_assets_version.find(asset);
            if(iter != m_assets_version.end())
            {
                iter->second.save_version = iter->second.edit_version;
            }
        }
        bool has_any_unsaved_changes() const
        {
            for(auto& asset : m_assets_version)
            {
                if(asset.second.edit_version != asset.second.save_version)
                {
                    return true;
                }
            }
            return false;
        }
        bool has_unsaved_changes(Asset::asset_t asset) const
        {
            auto iter = m_assets_version.find(asset);
            if(iter == m_assets_version.end())
            {
                return false;
            }
            return iter->second.edit_version != iter->second.save_version;
        }
        u32 get_asset_edit_version(Asset::asset_t asset) const
        {
            auto iter = m_assets_version.find(asset);
            if(iter == m_assets_version.end())
            {
                return 0;
            }
            return iter->second.edit_version;
        }

        //! Saves the specified asset.
        //! @param[in] asset The asset to save.
        RV save_asset(Asset::asset_t asset);

        //Ref<AssetBrowser> new_asset_browser(Path initial_path);
    };

    void register_components();

    void run_main_editor(const Path& project_path);

    void draw_asset_tile(Asset::asset_t asset, const RectF& draw_rect);

    extern MainEditor* g_main_editor;
}