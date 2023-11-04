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

namespace Luna
{
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

		MainEditor() :
			m_exiting(false),
			m_main_window_width(0),
			m_main_window_height(0) {}
		//m_next_asset_browser_index(0) {}

		RV init(const Path& project_path);

		RV update();

		void close();

		//Ref<AssetBrowser> new_asset_browser(Path initial_path);
	};

	void register_components();

	void run_main_editor(const Path& project_path);

	void draw_asset_tile(Asset::asset_t asset, const RectF& draw_rect);

	extern MainEditor* g_main_editor;
}