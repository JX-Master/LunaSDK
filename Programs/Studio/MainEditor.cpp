/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MainEditor.cpp
* @author JXMaster
* @date 2020/4/27
*/
#include "MainEditor.hpp"

#include "Assets/Texture.hpp"
#include "Assets/Mesh.hpp"
#include "Assets/Scene.hpp"
#include "Assets/Model.hpp"
#include "Assets/Material.hpp"

#include <VFS/VFS.hpp>
#include <Window/MessageBox.hpp>

#include "Camera.hpp"
#include "Light.hpp"
#include "SceneSettings.hpp"
#include "ModelRenderer.hpp"
#include <Runtime/Serialization.hpp>
#include "Scene.hpp"

#include "RenderPasses/SkyBoxPass.hpp"
#include "RenderPasses/ToneMappingPass.hpp"
#include "RenderPasses/WireframePass.hpp"
#include "RenderPasses/DepthPass.hpp"
#include "RenderPasses/GeometryPass.hpp"
#include "RenderPasses/DeferredLightingPass.hpp"
#include "RenderPasses/BufferVisualizationPass.hpp"
#include "RenderPasses/NormalVisualizationPass.hpp"

#include "SceneRenderer.hpp"
#include <Runtime/Log.hpp>

namespace Luna
{
	MainEditor* g_main_editor;

	RV MainEditor::init(const Path& project_path)
	{
		lutry
		{
			set_log_std_enabled(true);

			char title[256];
			auto name = project_path.filename();

			// Mount Data folder.
			auto mount_path = project_path;
			mount_path.push_back("Data");
			luexp(VFS::mount(VFS::get_platform_filesystem_driver(), mount_path.encode(PathSeparator::system_preferred).c_str(), "/"));

			// Load all asset metadata.
			luexp(Asset::update_assets_meta("/"));

			// Create window and render objects.
			snprintf(title, 256, "%s - Luna Studio", name.c_str());
			luset(m_window, Window::new_window(title, Window::WindowDisplaySettings::as_windowed(), Window::WindowCreationFlag::resizable));

			m_window->get_close_event() += [](Window::IWindow* window) {window->close(); };

			luset(m_swap_chain, RHI::new_swap_chain(g_env->graphics_queue, m_window, RHI::SwapChainDesc({0, 0, 2, RHI::Format::rgba8_unorm, true})));
			luset(m_cmdbuf, g_env->graphics_queue->new_command_buffer());

			// Create back buffer.
			//Ref<RHI::IResource> back_buffer;
			//u32 w = 0, h = 0;

			// Create ImGui context.
			ImGuiUtils::set_active_window(m_window);

			// Create asset browser instance.
			for (usize i = 0; i < 4; ++i)
			{
				Ref<AssetBrowser> browser = new_object<AssetBrowser>();
				browser->m_editor = this;
				//browser->m_index = m_next_asset_browser_index;
				//++m_next_asset_browser_index;
				browser->m_path = "/";
				auto his_path = browser->m_path;
				browser->m_histroy_paths.push_back(his_path);
				m_asset_browsers[i] = browser;
			}

			// Register types.

			register_components();

			luexp(register_static_texture_asset_type());
			register_texture_editor();
			register_texture_importer();
			register_static_mesh_asset_type();
			register_static_mesh_importer();
			
			register_material_asset_type();
			register_material_editor();
			register_model_asset_type();
			register_model_editor();

			register_scene_asset_type();
			luexp(register_scene_editor());

			g_env->new_asset_types.insert(get_material_asset_type());
			g_env->new_asset_types.insert(get_model_asset_type());
			g_env->new_asset_types.insert(get_scene_asset_type());

			luexp(register_sky_box_pass());
			luexp(register_wireframe_pass());
			luexp(register_depth_pass());
			luexp(register_geometry_pass());
			luexp(register_deferred_lighting_pass());
			luexp(register_tone_mapping_pass());
			luexp(register_buffer_visualization_pass());
			luexp(register_normal_visualization_pass());

			register_enum_type<SceneRendererMode>({
				luoption(SceneRendererMode, lit),
				luoption(SceneRendererMode, wireframe),
				luoption(SceneRendererMode, base_color),
				luoption(SceneRendererMode, normal),
				luoption(SceneRendererMode, roughness),
				luoption(SceneRendererMode, metallic),
				luoption(SceneRendererMode, depth),
				luoption(SceneRendererMode, emissive),
				luoption(SceneRendererMode, diffuse_lighting),
				luoption(SceneRendererMode, specular_lighting),
				luoption(SceneRendererMode, ambient_diffuse_lighting),
				luoption(SceneRendererMode, ambient_specular_lighting),
				luoption(SceneRendererMode, normal_visualization)
			});
		}
		lucatchret;
		return ok;
	}

	RV MainEditor::update()
	{
		Window::poll_events();

		if (m_window->is_closed())
		{
			m_exiting = true;
			return ok;
		}

		lutry
		{
			// Recreate the back buffer if needed.
			auto sz = m_window->get_size();
			if (sz.x && sz.y && (!m_back_buffer || sz.x != m_main_window_width || sz.y != m_main_window_height))
			{
				luexp(m_swap_chain->reset({sz.x, sz.y, 2, RHI::Format::unknown, true}));
				f32 clear_color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
				luset(m_back_buffer, RHI::get_main_device()->new_resource(RHI::ResourceDesc::tex2d(RHI::ResourceHeapType::local, RHI::Format::rgba8_unorm, 
					RHI::ResourceUsageFlag::render_target, sz.x, sz.y, 1, 1),
					&RHI::ClearValue::as_color(RHI::Format::rgba8_unorm, clear_color)));
				m_main_window_width = sz.x;
				m_main_window_height = sz.y;
				luset(m_back_buffer_rtv, RHI::get_main_device()->new_render_target_view(m_back_buffer));
			}

			ImGuiUtils::update_io();
			ImGui::NewFrame();
			ImGuizmo::BeginFrame();

			// Main window GUI code goes here.

			//m_ctx->show_demo_window();

			// Dock space.
			ImGui::SetNextWindowPos({ 0.0f, 0.0f });
			ImGui::SetNextWindowSize({ (f32)m_main_window_width, (f32)m_main_window_height });
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
			ImGui::Begin("DockSpace", nullptr,  ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoDocking);

			ImGui::DockSpace(ImGui::GetID("DockSpace Context"));

			ImGui::End();
			ImGui::PopStyleVar(3);

			// Draw Asset Browser.
			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("View"))
				{
					for (usize i = 0; i < 4; ++i)
					{
						c8 buf[32];
						snprintf(buf, 32, "Asset Browser %u", (u32)i);
						ImGui::Checkbox(buf, &m_asset_browsers_enabled[i]);
					}
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}

			for (usize i = 0; i < 4; ++i)
			{
				if (m_asset_browsers_enabled[i])
				{
					m_asset_browsers[i]->render();
				}
			}

			// Draw Editors.
			auto iter = m_editors.begin();
			while (iter != m_editors.end())
			{
				if ((*iter)->closed())
				{
					iter = m_editors.erase(iter);
				}
				else
				{
					(*iter)->on_render();
					++iter;
				}
			}

			ImGui::Render();
			RHI::RenderPassDesc render_pass;
			render_pass.color_attachments[0] = m_back_buffer_rtv;
			render_pass.color_load_ops[0] = RHI::LoadOp::clear;
			render_pass.color_clear_values[0] = { 0.0f, 0.0f, 0.0f, 1.0f };
			m_cmdbuf->begin_render_pass(render_pass);
			m_cmdbuf->end_render_pass();
			luexp(ImGuiUtils::render_draw_data(ImGui::GetDrawData(), m_cmdbuf, m_back_buffer_rtv));
			luexp(m_cmdbuf->submit());
			m_cmdbuf->wait();
			luexp(m_cmdbuf->reset());
			luexp(m_swap_chain->present(m_back_buffer, 0));
			m_swap_chain->wait();
		}
		lucatchret;
		return ok;
	}

	void register_components()
	{
		register_enum_type<CameraType>({
				luoption(CameraType, perspective),
				luoption(CameraType, orthographic)
			});
		set_serializable<CameraType>();
		
		register_struct_type<Camera>({
			luproperty(Camera, CameraType, type),
			luproperty(Camera, f32, fov),
			luproperty(Camera, f32, size),
			luproperty(Camera, f32, near_clipping_plane),
			luproperty(Camera, f32, far_clipping_plane),
			luproperty(Camera, f32, aspect_ratio)
			});
		set_serializable<Camera>();
		g_env->component_types.insert(typeof<Camera>());
		set_property_attribute(typeof<Camera>(), "fov", "radian", true);
		set_property_attribute(typeof<Camera>(), "fov", "gui_min", (f64)deg_to_rad(60));
		set_property_attribute(typeof<Camera>(), "fov", "gui_max", (f64)deg_to_rad(160));
		set_property_attribute(typeof<Camera>(), "aspect_ratio", "hide", true);

		register_struct_type<DirectionalLight>({
				luproperty(DirectionalLight, Float3, intensity),
				luproperty(DirectionalLight, f32, intensity_multiplier)
			});
		set_serializable<DirectionalLight>();
		g_env->component_types.insert(typeof<DirectionalLight>());

		register_struct_type<PointLight>({
			luproperty(PointLight, Float3, intensity),
			luproperty(PointLight, f32, intensity_multiplier),
			luproperty(PointLight, f32, attenuation_power)
			});
		set_serializable<PointLight>();
		g_env->component_types.insert(typeof<PointLight>());

		register_struct_type<SpotLight>({
			luproperty(SpotLight, Float3, intensity),
			luproperty(SpotLight, f32, intensity_multiplier),
			luproperty(SpotLight, f32, attenuation_power),
			luproperty(SpotLight, f32, spot_power)
			});
		set_serializable<SpotLight>();
		g_env->component_types.insert(typeof<SpotLight>());

		set_property_attribute(typeof<DirectionalLight>(), "intensity", "color_gui", true);
		set_property_attribute(typeof<PointLight>(), "intensity", "color_gui", true);
		set_property_attribute(typeof<SpotLight>(), "intensity", "color_gui", true);

		register_struct_type<ModelRenderer>({
			luproperty(ModelRenderer, Asset::asset_t, model)
			});
		set_serializable<ModelRenderer>();
		g_env->component_types.insert(typeof<ModelRenderer>());

		register_struct_type<SceneSettings>({
			luproperty(SceneSettings, Name, camera_entity),
			luproperty(SceneSettings, Asset::asset_t, skybox),
			luproperty(SceneSettings, Float3, environment_color),
			luproperty(SceneSettings, f32, skybox_rotation),
			luproperty(SceneSettings, f32, exposure),
			luproperty(SceneSettings, bool, auto_exposure)
			});
		set_serializable<SceneSettings>();
		g_env->scene_component_types.insert(typeof<SceneSettings>());
		set_property_attribute(typeof<SceneSettings>(), "environment_color", "color_gui", true);
		set_property_attribute(typeof<SceneSettings>(), "exposure", "gui_min", (f64)0.00001f);
		set_property_attribute(typeof<SceneSettings>(), "exposure", "gui_max", (f64)1.0f);
	}

	void run_main_editor(const Path& project_path)
	{
		register_boxed_type<MainEditor>();
		register_boxed_type<AssetBrowser>();

		Ref<MainEditor> main_editor = new_object<MainEditor>();
		g_main_editor = main_editor;
		if (!main_editor)
		{
			return;
		}
		lutry
		{
			luexp(main_editor->init(project_path));
			while (!main_editor->m_exiting)
			{
				luexp(main_editor->update());
			}
		}
		lucatch
		{
			auto _ = Window::message_box(explain(lures), "Editor Crashed.", Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
			return;
		}
	}

	void draw_asset_tile(Asset::asset_t asset, const RectF& draw_rect)
	{
		if (asset)
		{
			auto asset_type = Asset::get_asset_type(asset);
			auto iter = g_env->editor_types.find(asset_type);
			if (iter != g_env->editor_types.end())
			{
				if (iter->second.on_draw_tile)
				{
					iter->second.on_draw_tile(iter->second.userdata.get(), asset, draw_rect);
				}
				else
				{
					// Draw default tile.
					auto text_sz = ImGui::CalcTextSize(asset_type.c_str());
					Float2 center = Float2(draw_rect.offset_x + draw_rect.width / 2.0f, draw_rect.offset_y + draw_rect.height / 2.0f);
					ImGui::SetCursorPos({ center.x - text_sz.x / 2.0f, center.y - text_sz.y / 2.0f });
					ImGui::Text(asset_type.c_str());
				}
				return;
			}
			auto text_sz = ImGui::CalcTextSize(asset_type.c_str());
			Float2 center = Float2(draw_rect.offset_x + draw_rect.width / 2.0f, draw_rect.offset_y + draw_rect.height / 2.0f);
			ImGui::SetCursorPos(center - text_sz / 2.0f);
			ImGui::Text(asset_type.c_str());
		}
	}
}