/*
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
#include <Runtime/HashMap.hpp>
#include "StudioHeader.hpp"

namespace Luna
{
	struct MainEditor
	{
		lustruct("MainEditor", "{CF004929-E981-4E1D-A4AE-96EEC79AD1EB}");

		Path m_project_path;

		Ref<Window::IWindow> m_window;
		Ref<RHI::ISwapChain> m_swap_chain;
		Ref<RHI::ICommandBuffer> m_cmdbuf;
		Ref<RHI::IResource> m_back_buffer;
		Ref<RHI::IRenderTargetView> m_back_buffer_rtv;

		Ref<AssetBrowser> m_asset_browser;

		Vector<Ref<IAssetEditor>> m_editors;

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

		//Ref<AssetBrowser> new_asset_browser(Path initial_path);
	};

	void register_components();

	void run_main_editor(const Path& project_path);

	void draw_asset_tile(Asset::asset_t asset, const RectF& draw_rect);

	extern MainEditor* g_main_editor;
}