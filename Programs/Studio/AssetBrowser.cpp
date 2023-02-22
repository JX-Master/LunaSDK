/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AssetBrowser.cpp
* @author JXMaster
* @date 2020/4/29
*/
#include "AssetBrowser.hpp"
#include "MainEditor.hpp"
#include <Runtime/Debug.hpp>
#include <Runtime/Math/Vector.hpp>
#include <Runtime/Math/Color.hpp>
#include <Runtime/Log.hpp>

namespace Luna
{
	struct AssetThumbnail
	{
		Name m_filename;	// Without extension.
		bool m_is_dir;
	};

	R<Vector<AssetThumbnail>> get_assets_in_folder(const Path& folder_path)
	{
		Vector<AssetThumbnail> assets;
		lutry
		{
			lulet(iter, VFS::open_dir(folder_path));
			while (iter->valid())
			{
				if ((iter->attribute() & FileAttributeFlag::directory) != FileAttributeFlag::none)
				{
					// This is a directory.
					if (!strcmp(iter->filename(), ".") || !strcmp(iter->filename(), ".."))
					{
						iter->move_next();
						continue;
					}
					AssetThumbnail t;
					t.m_filename = Name(iter->filename());
					t.m_is_dir = true;
					assets.push_back(t);
				}
				else
				{
					// Ends with ".meta.la" or ".meta.lb"
					const char* name = iter->filename();
					usize name_len = strlen(name);
					if (name_len > 5)
					{
						if (!strcmp(name + name_len - 5, ".meta"))
						{
							AssetThumbnail t;
							t.m_filename = Name(name, name_len - 5);
							t.m_is_dir = false;
							assets.push_back(t);
						}
					}
				}
				iter->move_next();
			}
		}
		lucatchret;
		return assets;
	}

	void AssetBrowser::change_path(const Path& path)
	{
		if (m_current_location_in_histroy_path != m_histroy_paths.size() - 1)
		{
			// Clear forwards.
			m_histroy_paths.resize(m_current_location_in_histroy_path + 1);
		}
		m_path.assign(path);
		m_histroy_paths.push_back(m_path);
		++m_current_location_in_histroy_path;
	}

	void AssetBrowser::render()
	{
		char title[64];
		sprintf_s(title, "Asset Browser");

		ImGui::SetNextWindowSize({ 1000.0f, 500.0f }, ImGuiCond_FirstUseEver);
		ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("New"))
			{
				for (auto& i : g_env->importer_types)
				{
					auto name = i.first;
					if (ImGui::MenuItem(name.c_str()))
					{
						auto editor = i.second.new_importer(m_path);
						m_editor->m_editors.push_back(editor);
					}
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		navbar();

		tile_context();

		ImGui::End();
	}
	void AssetBrowser::navbar()
	{
		// Draw back/forward/pop arrow.
		bool back_disabled = (m_current_location_in_histroy_path == 0);
		if (back_disabled)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		if (ImGui::ArrowButton("back", ImGuiDir_Left) && !back_disabled)
		{
			--m_current_location_in_histroy_path;
			m_path.assign(m_histroy_paths[m_current_location_in_histroy_path]);
		}
		if (back_disabled)
		{
			ImGui::PopStyleVar();
		}
		ImGui::SameLine();
		bool forward_disabled = (m_current_location_in_histroy_path == m_histroy_paths.size() - 1);
		if (forward_disabled)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		if (ImGui::ArrowButton("forward", ImGuiDir_Right) && !forward_disabled)
		{
			++m_current_location_in_histroy_path;
			m_path.assign(m_histroy_paths[m_current_location_in_histroy_path]);
		}
		if (forward_disabled)
		{
			ImGui::PopStyleVar();
		}
		ImGui::SameLine();
		bool pop_disabled = m_path.empty();
		if (pop_disabled)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		if (ImGui::ArrowButton("pop", ImGuiDir_Up) && !pop_disabled)
		{
			auto path = m_path;
			path.pop_back();
			change_path(path);
		}
		if (pop_disabled)
		{
			ImGui::PopStyleVar();
		}
		ImGui::SameLine();
		// Draw path.
		{
			Float2 pos = ImGui::GetCursorScreenPos();
			Float2 frame_padding = ImGui::GetStyle().FramePadding;

			Float2 region_min = pos;
			Float2 region_max = pos + frame_padding * 2 + Float2(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().WindowPadding.x,
				ImGui::GetTextLineHeight());
			if (!m_is_text_editing)
			{
				auto dl = ImGui::GetWindowDrawList();
				dl->AddRectFilled(region_min, region_max, 0xFF202020);

				dl->AddRect(region_min, region_max,
					Color(ImGui::GetStyle().Colors[(u32)ImGuiCol_Border]).abgr8());

				bool btn_clicked = false;
				if ((m_path.flags() & PathFlag::absolute) != PathFlag::none)
				{
					ImGui::Text("/");
					if (m_path.size())
					{
						ImGui::SameLine();
					}
				}
				Float2 mouse_pos = ImGui::GetIO().MousePos;
				Path changed_path;
				for (u32 i = 0; i < m_path.size(); ++i)
				{
					auto node = m_path[i];
					ImGui::PushID(i);
					if (ImGui::Button(node.c_str()) && i != (m_path.size() - 1))
					{
						changed_path = m_path;
						for (u32 j = i; j < m_path.size() - 1; ++j)
						{
							changed_path.pop_back();
						}
					}
					auto btn_min = ImGui::GetItemRectMin();
					auto btn_max = ImGui::GetItemRectMax();
					if (in_bounds(mouse_pos, btn_min, btn_max) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						btn_clicked = true;
					}
					ImGui::PopID();
					ImGui::SameLine();
					ImGui::Text("/");
					if ((m_path.size() > 1) && (i != m_path.size() - 1))
					{
						ImGui::SameLine();
					}
				}
				if (!changed_path.empty())
				{
					change_path(changed_path);
				}


				if (in_bounds(mouse_pos, region_min, region_max) && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !btn_clicked)
				{
					// Switch to text mode.
					m_is_text_editing = true;
					m_path_edit_text = m_path.encode(PathSeparator::slash, true);
				}
			}
			else
			{
				ImGui::SetNextItemWidth(region_max.x - region_min.x);
				ImGui::InputText("##PathTextEditing", m_path_edit_text);
				auto mouse_pos = ImGui::GetIO().MousePos;
				if (!in_bounds(mouse_pos, region_min, region_max) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					// Switch to normal mode.
					m_is_text_editing = false;
					auto new_p = Path(m_path_edit_text.c_str());
					auto attr = VFS::file_attribute(new_p);
					if (succeeded(attr) && ((attr.get().attributes & FileAttributeFlag::directory) != FileAttributeFlag::none))
					{
						m_path.assign(new_p);
					}
				}
			}
		}

	}

	void AssetBrowser::tile_context()
	{
		// Draw content.
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
		ImGui::BeginChild("ctx", Float2(0.0f, 0.0f), true, ImGuiWindowFlags_NoMove);
		auto assets = get_assets_in_folder(m_path);
		Float2 mouse_pos = ImGui::GetIO().MousePos;
		if (succeeded(assets))
		{
			if (assets.get().empty())
			{
				auto region = ImGui::GetContentRegionAvail();
				auto region_center = region / 2;
				const char* text = "Empty Directory";
				auto text_size = ImGui::CalcTextSize(text);
				ImGui::SetCursorPos(region_center - Float2(text_size.x / 2, text_size.y / 2));
				ImGui::Text(text);
			}
			else
			{
				// Draw asset tiles.

				usize num_assets = assets.get().size();

				constexpr u32 padding = 5;

				u32 tile_width = (u32)(m_tile_size + padding * 2);
				u32 tile_height = (u32)(m_tile_size + padding * 2 + ImGui::GetTextLineHeight());
				auto window_pos = ImGui::GetWindowPos();

				f32 woff = 0;
				f32 hoff = 0;
				auto origin_pos = ImGui::GetCursorScreenPos();

				for (usize i = 0; i < num_assets; ++i)
				{
					auto dl = ImGui::GetWindowDrawList();

					auto tile_min = ImGui::GetCursorScreenPos() + padding;
					auto tile_max = tile_min + Float2((f32)tile_width, (f32)tile_height);

					auto siter = m_selections.find(assets.get()[i].m_filename);
					if (siter != m_selections.end())
					{
						// Draw selection background.
						dl->AddRectFilled(tile_min - padding, tile_max - padding, Color(ImGui::GetStyle().Colors[(u32)ImGuiCol_Button]).abgr8());
					}

					if (assets.get()[i].m_is_dir)
					{
						auto folder_icon_begin_pos = ImGui::GetCursorScreenPos() + Float2(padding, padding);
						// Draw an folder icon.
						Float2 shape1[4] = {
							{41.1f, 21.0f},
							{85.3f, 21.0f},
							{91.5f, 33.5f},
							{35.5f, 33.5f},
						};
						Float2 shape2[4] = {
							{9.9f, 36.3f},
							{91.5f, 36.3f},
							{80.5f, 90.4f},
							{19.6f, 90.4f}
						};
						dl->PathClear();
						dl->PathLineTo(shape1[0] * m_tile_size / 100.0f + folder_icon_begin_pos);
						dl->PathLineTo(shape1[1] * m_tile_size / 100.0f + folder_icon_begin_pos);
						dl->PathLineTo(shape1[2] * m_tile_size / 100.0f + folder_icon_begin_pos);
						dl->PathLineTo(shape1[3] * m_tile_size / 100.0f + folder_icon_begin_pos);
						dl->PathFillConvex(0xFFCCCCCC);
						dl->PathClear();
						dl->PathLineTo(shape2[0] * m_tile_size / 100.0f + folder_icon_begin_pos);
						dl->PathLineTo(shape2[1] * m_tile_size / 100.0f + folder_icon_begin_pos);
						dl->PathLineTo(shape2[2] * m_tile_size / 100.0f + folder_icon_begin_pos);
						dl->PathLineTo(shape2[3] * m_tile_size / 100.0f + folder_icon_begin_pos);
						dl->PathFillConvex(0xFFCCCCCC);

						if (mouse_pos.x > tile_min.x && mouse_pos.y > tile_min.y && mouse_pos.x < tile_max.x && mouse_pos.y < tile_max.y)
						{
							if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
							{
								// Change path.
								auto path = m_path;
								path.push_back(assets.get()[i].m_filename);
								change_path(path);
							}
						}
					}
					else
					{
						auto meta_path = m_path;
						meta_path.push_back(assets.get()[i].m_filename);
						auto asset = Asset::get_asset_by_path(meta_path);
						if (succeeded(asset))
						{
							auto draw_rect = RectF(tile_min.x - window_pos.x, tile_min.y - window_pos.y, m_tile_size, m_tile_size);

							ImGui::SetCursorPos({ draw_rect.offset_x, draw_rect.offset_y });
							ImGui::PushID(asset.get().handle);
							ImGui::Button("", { draw_rect.width, draw_rect.height });
							ImGui::PopID();

							if (ImGui::BeginDragDropSource())
							{
								Asset::asset_t payload = asset.get();
								ImGui::SetDragDropPayload("Asset Ref", &payload, sizeof(payload));
								ImGui::Text(meta_path.encode().c_str());
								ImGui::EndDragDropSource();
							}

							// Editor logic.
							auto asset_type = Asset::get_asset_type(asset.get());
							auto iter = g_env->editor_types.find(asset_type);

							if (iter != g_env->editor_types.end())
							{
								if (iter->second.on_draw_tile)
								{
									iter->second.on_draw_tile(iter->second.userdata.get(), asset.get(), draw_rect);
								}
								else
								{
									// Draw default tile.
									auto text_sz = ImGui::CalcTextSize(asset_type.c_str());
									Float2 center = Float2(draw_rect.offset_x + draw_rect.width / 2.0f, draw_rect.offset_y + draw_rect.height / 2.0f);
									ImGui::SetCursorPos({ center.x - text_sz.x / 2.0f, center.y - text_sz.y / 2.0f });
									ImGui::Text(asset_type.c_str());
								}

								if (mouse_pos.x > tile_min.x && mouse_pos.y > tile_min.y && mouse_pos.x < tile_max.x && mouse_pos.y < tile_max.y)
								{
									if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
									{
										// Open Editor.
										auto edit = iter->second.new_editor(iter->second.userdata.get(), asset.get());
										m_editor->m_editors.push_back(edit);
									}
								}
							}
							else
							{
								auto text_sz = ImGui::CalcTextSize(asset_type.c_str());
								Float2 center = Float2(draw_rect.offset_x + draw_rect.width / 2.0f, draw_rect.offset_y + draw_rect.height / 2.0f);
								ImGui::SetCursorPos(center - text_sz / 2.0f);
								ImGui::Text(asset_type.c_str());
							}

							// Load the data if not loaded.
							if (Asset::get_asset_state(asset.get()) == Asset::AssetState::unloaded)
							{
								auto& err = Asset::get_asset_loading_result(asset.get());
								if (err.code != ErrCode(0))
								{
									// Output error.
									log_error("App", "Asset Loading Error: %s", err.explain());
								}
								Asset::load_asset(asset.get());
							}

							// Draw status circle.
							if (Asset::get_asset_state(asset.get()) == Asset::AssetState::loaded)
							{
								// Draw green circle.
								dl->AddCircleFilled(tile_min + Float2(m_tile_size, m_tile_size) - 5.0f, 10.0f, Color::green().abgr8());
							}
							else
							{
								// Draw yellow circle.
								dl->AddCircleFilled(tile_min + Float2(m_tile_size, m_tile_size) - 5.0f, 10.0f, Color::yellow().abgr8());
							}
						}
						else
						{
							auto text_sz = ImGui::CalcTextSize("Unknown");
							Float2 center = tile_min / 2.0f + (tile_min + Float2(m_tile_size, m_tile_size) / 2.0f);
							ImGui::SetCursorPos(center - text_sz / 2.0f);
							ImGui::Text("Unknown");
							dl->AddCircleFilled(tile_min + Float2(m_tile_size, m_tile_size) - 5.0f, 10.0f, Color::red().abgr8());
						}
					}

					// Draw asset name.
					ImGui::SetCursorPos(Float2(tile_min.x - window_pos.x, tile_min.y - window_pos.y + m_tile_size));
					ImGui::Text(assets.get()[i].m_filename.c_str());

					// Check if the asset is clicked / double clicked.

					if (mouse_pos.x > tile_min.x && mouse_pos.y > tile_min.y && mouse_pos.x < tile_max.x && mouse_pos.y < tile_max.y)
					{
						if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
						{
							m_selections.clear();
							m_selections.insert(assets.get()[i].m_filename);
						}
					}

					// Update woff and hoff.
					woff += tile_width;
					if (woff + tile_width > ImGui::GetWindowWidth())
					{
						woff = 0;
						hoff += tile_height;
					}

					// Set cursor pos for next tile.
					ImGui::SetCursorScreenPos(origin_pos + Float2(woff, hoff));
				}
			}
		}
		else
		{
			auto region = ImGui::GetContentRegionAvail();
			auto region_center = region / 2;
			const char* text_fail = "Failed to display assets in this directory.";
			const char* text_reason = explain(assets.errcode());
			auto text_fail_size = ImGui::CalcTextSize(text_fail);
			auto text_reason_size = ImGui::CalcTextSize(text_reason);
			ImGui::SetCursorPos(region_center - Float2(text_fail_size.x / 2, text_fail_size.y));
			ImGui::Text(text_fail);
			ImGui::SetCursorPos(region_center - Float2(text_reason_size.x / 2, 0.0f));
			ImGui::Text(text_reason);
		}

		auto tile_ctx_min = ImGui::GetWindowPos();
		auto tile_ctx_max = ImGui::GetWindowPos() + ImGui::GetWindowSize();

		//if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && in_bounds(mouse_pos, tile_ctx_min, tile_ctx_max))
		//{
		//	m_ms_mouse_begin_pos = mouse_pos;
		//	m_ms_is_dragging = true;
		//}

		//if (m_ms_is_dragging)
		//{
		//	// Draw drag rect.
		//	auto dl = ImGui::GetWindowDrawList();
		//	auto rect_min = min(mouse_pos, Float2(m_ms_mouse_begin_pos));
		//	auto rect_max = max(mouse_pos, Float2(m_ms_mouse_begin_pos));

		//	auto color = ImGui::GetStyle().Colors[(u32)ImGuiCol_Button];
		//	color.w *= 0.5f;
		//	dl->AddRectFilled(rect_min, rect_max, Color(color).abgr8());

		//	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		//	{
		//		m_ms_is_dragging = false;
		//	}
		//}

		ImGui::EndChild();
		ImGui::PopStyleVar();
	}
}