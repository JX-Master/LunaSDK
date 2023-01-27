/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SceneEditor.cpp
* @author JXMaster
* @date 2020/5/15
*/
#include "Scene.hpp"
#include "../MainEditor.hpp"
#include <Runtime/Debug.hpp>
#include "../Scene.hpp"
#include <ShaderCompiler/ShaderCompiler.hpp>
#include <RHI/ShaderCompileHelper.hpp>
#include <Runtime/Math/Color.hpp>
#include <Window/MessageBox.hpp>
#include "../Light.hpp"
#include "../Camera.hpp"
#include "../ModelRenderer.hpp"
#include "../SceneRenderer.hpp"
#include "../Model.hpp"
#include <HID/Mouse.hpp>
#include "../EditObject.hpp"
#include "../Mesh.hpp"
#include "../Material.hpp"
namespace Luna
{
	struct CameraCB
	{
		Float4x4 world_to_view;
		Float4x4 view_to_proj;
		Float4x4 world_to_proj;
		Float4x4 view_to_world;
		Float4 env_color;
	};

	struct LightingParams
	{
		Float3U strength;
		f32 attenuation_power;
		Float3U direction;
		u32 type;
		Float3U position;
		f32 spot_attenuation_power;
	};

	struct ToneMappingParams
	{
		f32 exposure = 1.0f;
	};

	struct SkyboxParams
	{
		Float4x4 view_to_world;
		f32 fov;
		u32 width;
		u32 height;
	};

	struct SceneEditorUserData
	{
		lustruct("SceneEditorUserData", "{5b4aea33-e61a-4042-ba91-1f4ec84f8194}");

		// Resources for rendering grids.
		Ref<RHI::IResource> m_grid_vb;
		Ref<RHI::IDescriptorSetLayout> m_grid_dlayout;
		Ref<RHI::IShaderInputLayout> m_grid_slayout;
		Ref<RHI::IPipelineState> m_grid_pso;

		// Resources for rendering debug meshes.
		Ref<RHI::IPipelineState> m_debug_mesh_renderer_pso;
		Ref<RHI::IDescriptorSetLayout> m_debug_mesh_renderer_dlayout;
		Ref<RHI::IShaderInputLayout> m_debug_mesh_renderer_slayout;

		// Depth Pass.
		Ref<RHI::IPipelineState> m_depth_pass_pso;
		Ref<RHI::IDescriptorSetLayout> m_depth_pass_dlayout;
		Ref<RHI::IShaderInputLayout> m_depth_pass_slayout;

		// Sky box Pass.
		Ref<RHI::IPipelineState> m_skybox_pass_pso;
		Ref<RHI::IDescriptorSetLayout> m_skybox_pass_dlayout;
		Ref<RHI::IShaderInputLayout> m_skybox_pass_slayout;

		// Lighting Pass.
		Ref<RHI::IPipelineState> m_lighting_pass_pso;
		Ref<RHI::IDescriptorSetLayout> m_lighting_pass_dlayout;
		Ref<RHI::IShaderInputLayout> m_lighting_pass_slayout;

		Ref<RHI::IResource> m_default_base_color;	// 1.0f, 1.0f, 1.0f, 1.0f
		Ref<RHI::IResource> m_default_roughness;	// 0.5f
		Ref<RHI::IResource> m_default_normal;		// 0.5f, 0.5f, 1.0f, 1.0f
		Ref<RHI::IResource> m_default_metallic;	// 0.0f
		Ref<RHI::IResource> m_default_emissive;	// 0.0f, 0.0f, 0.0f, 0.0f

		// Tone mapping pass.
		Ref<RHI::IDescriptorSetLayout> m_first_lum_pass_dlayout;
		Ref<RHI::IShaderInputLayout> m_first_lum_pass_slayout;
		Ref<RHI::IPipelineState> m_first_lum_pass_pso;
		Ref<RHI::IDescriptorSetLayout> m_lum_pass_dlayout;
		Ref<RHI::IShaderInputLayout> m_lum_pass_slayout;
		Ref<RHI::IPipelineState> m_lum_pass_pso;
		Ref<RHI::IDescriptorSetLayout> m_tone_mapping_pass_dlayout;
		Ref<RHI::IShaderInputLayout> m_tone_mapping_pass_slayout;
		Ref<RHI::IPipelineState> m_tone_mapping_pass_pso;

		SceneEditorUserData() {}

		RV init();
	};

	struct SceneEditor : public IAssetEditor
	{
	public:
		lustruct("SceneEditor", "{c973cc28-78e7-4be5-a391-8c2e5960fa48}");
		luiimpl();

		Ref<SceneEditorUserData> m_type;

		Asset::asset_t m_scene;

		// States for entity list.
		i32 m_new_entity_current_item = 0;
		u32 m_current_select_entity = 0;
		bool m_name_editing = false;
		String m_name_editing_buf = String();

		// States for component grid.
		WeakRef<Entity> m_current_entity;

		// States for scene viewport.
		CameraCB m_camera_cb_data;
		Ref<RHI::IResource> m_camera_cb;
		Ref<RHI::ICommandBuffer> m_scene_cmdbuf;

		Ref<RHI::IDescriptorSet> m_grid_desc_set;

		Ref<RHI::IResource> m_model_matrices;
		usize m_num_model_matrices = 0;

		Ref<RHI::IResource> m_skybox_params;

		Ref<RHI::IResource> m_lighting_params;
		usize m_num_lights = 0;

		Ref<RHI::IResource> m_tone_mapping_offset;
		Ref<RHI::IResource> m_tone_mapping_params;

		ImGui::GizmoMode m_gizmo_mode = ImGui::GizmoMode::local;
		ImGui::GizmoOperation m_gizmo_op = ImGui::GizmoOperation::translate;

		f32 m_camera_speed = 1.0f;

		bool m_wireframe = false;
		bool m_grid = true;

		bool m_navigating = false;
		Int2U m_scene_click_pos;	// Stores the click mouse position in screen space.

		bool m_open = true;

		SceneEditor() {}

		RV init();

		void draw_entity_list();
		void draw_scene_components_grid();
		RV draw_scene();

		void draw_components_grid();

		virtual void on_render() override;
		virtual bool closed() override
		{
			return !m_open;
		}
	};

	RV SceneEditor::init()
	{
		lutry
		{
			using namespace RHI;
			auto device = get_main_device();
			u32 cb_align = device->get_constant_buffer_data_alignment();
			luset(m_camera_cb, device->new_resource(ResourceDesc::buffer(ResourceHeapType::upload, ResourceUsageFlag::constant_buffer, align_upper(sizeof(CameraCB), cb_align))));
			luset(m_scene_cmdbuf, g_env->graphics_queue->new_command_buffer());

			luset(m_grid_desc_set, device->new_descriptor_set(DescriptorSetDesc(m_type->m_grid_dlayout)));
			m_grid_desc_set->set_cbv(0, m_camera_cb, ConstantBufferViewDesc(0, (u32)align_upper(sizeof(CameraCB), cb_align)));
			/*m_back_buffer_clear_pass = device->new_render_pass(
				RenderPassDesc({ AttachmentDesc(Format::rgba8_unorm, EAttachmentLoadOp::clear, EAttachmentStoreOp::store) },
					Format::d32_float, EAttachmentLoadOp::clear, EAttachmentStoreOp::store, EAttachmentLoadOp::dont_care, EAttachmentStoreOp::dont_care, 1, true)).get();*/

			luset(m_lighting_params, device->new_resource(ResourceDesc::buffer(ResourceHeapType::upload, ResourceUsageFlag::shader_resource, align_upper(sizeof(LightingParams) * 4, cb_align))));
			m_num_lights = 4;

			luset(m_tone_mapping_offset, device->new_resource(ResourceDesc::buffer(ResourceHeapType::upload, ResourceUsageFlag::constant_buffer, align_upper(sizeof(Float4) * 16, cb_align))));

			luset(m_tone_mapping_params, device->new_resource(ResourceDesc::buffer(ResourceHeapType::upload, ResourceUsageFlag::constant_buffer, align_upper(sizeof(ToneMappingParams), cb_align))));

			luset(m_skybox_params, device->new_resource(ResourceDesc::buffer(ResourceHeapType::upload, ResourceUsageFlag::constant_buffer, align_upper(sizeof(SkyboxParams), cb_align))));
		}
		lucatchret;
		return ok;
	}

	void SceneEditor::draw_entity_list()
	{
		auto s = Asset::get_asset_data<Scene>(m_scene);

		// Draw entity list.
		ImGui::Text("Entity List");

		ImGui::SameLine();

		if (ImGui::Button("New Entity"))
		{
			char name[64];
			strcpy_s(name, "New_Entity");
			auto entity = s->add_entity(Name(name));
			if (entity.errcode() == BasicError::already_exists())
			{
				u32 index = 0;
				// Append index.
				while (failed(entity))
				{
					sprintf_s(name, "New_Entity_%u", index);
					entity = s->add_entity(Name(name));
					++index;
				}
			}
		}

		auto avail = ImGui::GetContentRegionAvail();

		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
		ImGui::BeginChild("Entity List", Float2(avail.x, avail.y / 2.0f), true);

		if (s->root_entities.empty())
		{
			ImGui::Text("No entity in the scene.");
		}
		else
		{
			const char* entity_popup_id = "Entity Popup";

			Float2 sel_size{ ImGui::GetWindowWidth(), ImGui::GetTextLineHeight() };

			auto& entities = s->root_entities;

			for (u32 i = 0; i < (u32)entities.size(); ++i)
			{
				Float2 sel_pos = ImGui::GetCursorScreenPos();
				if (in_rect(ImGui::GetIO().MousePos, sel_pos, sel_pos + sel_size) && !ImGui::IsPopupOpen(entity_popup_id) &&
					(ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
				{
					m_current_select_entity = i;
					m_current_entity = entities[i];
				}
				if (i == m_current_select_entity && m_name_editing)
				{
					ImGui::InputText("###NameEdit", m_name_editing_buf);
					if (!in_rect(ImGui::GetIO().MousePos, sel_pos, sel_pos + sel_size) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						for (usize i = 0; i < m_name_editing_buf.size(); ++i)
						{
							if (m_name_editing_buf.data()[i] == ' ')
							{
								m_name_editing_buf.data()[i] = '_';
							}
						}
						entities[i]->name = m_name_editing_buf.c_str();
						m_name_editing = false;
					}
				}
				else
				{
					// Draw highlight.
					if (i == m_current_select_entity)
					{
						auto dl = ImGui::GetWindowDrawList();
						dl->AddRectFilled(sel_pos, sel_pos + sel_size,
							Color(ImGui::GetStyle().Colors[(u32)ImGuiCol_Button]).abgr8());
					}
					ImGui::Text(entities[i]->name.c_str());
				}

				if (in_rect(ImGui::GetIO().MousePos, sel_pos, sel_pos + sel_size) && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					ImGui::OpenPopup(entity_popup_id);
				}

			}

			if (ImGui::BeginPopup(entity_popup_id))
			{
				if (ImGui::Selectable("Rename"))
				{
					m_name_editing = true;
					m_name_editing_buf.assign(s->root_entities[m_current_select_entity]->name.c_str());
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Selectable("Remove"))
				{
					s->root_entities.erase(s->root_entities.begin() + m_current_select_entity);
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

		ImGui::EndChild();
		ImGui::PopStyleVar();
	}
	void SceneEditor::draw_scene_components_grid()
	{
		ImGui::Text("Scene Components");

		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
		ImGui::BeginChild("Scene Components", Float2(0.0f, 0.0f), true);

		auto s = Asset::get_asset_data<Scene>(m_scene);
		auto& components = s->scene_components;
		if (components.empty())
		{
			ImGui::Text("No Components");
		}
		else
		{
			auto iter = components.begin();
			while (iter != components.end())
			{
				if (ImGui::CollapsingHeader(get_type_name(iter->first).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					edit_object(iter->second.get());
					
					ImGui::PushID(iter->first);

					if (ImGui::Button("Remove"))
					{
						iter = components.erase(iter);
					}
					else
					{
						++iter;
					}

					ImGui::PopID();
				}
			}
		}

		const char* new_comp_popup = "NewSceneCompPopup";

		if (ImGui::Button("New Scene Component"))
		{
			ImGui::OpenPopup(new_comp_popup);
		}

		if (ImGui::BeginPopup(new_comp_popup))
		{

			for (auto& i : g_env->scene_component_types)
			{
				auto name = get_type_name(i);
				auto exists = s->scene_components.contains(i);
				if (!exists)
				{
					// Show enabled.
					if (ImGui::Selectable(name.c_str()))
					{
						object_t comp = object_alloc(i);
						construct_type(i, comp);
						ObjRef comp_obj;
						comp_obj.attach(comp);
						components.insert(make_pair(i, move(comp_obj)));
						ImGui::CloseCurrentPopup();
					}
				}
				else
				{
					// Show disabled.
					ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_Disabled);
				}
			}
			ImGui::EndPopup();
		}

		ImGui::EndChild();

		ImGui::PopStyleVar();
	}

	RV SceneEditor::draw_scene()
	{
		lutry
		{
			ImGui::Text("Scene");

			auto s = Asset::get_asset_data<Scene>(m_scene);

			auto scene_renderer = s->get_scene_component<SceneRenderer>();
			if (!scene_renderer)
			{
				ImGui::Text("Please add Scene Renderer Component to the scene.");
				return ok;
			}

			if (!scene_renderer->screen_buffer)
			{
				Float2 scene_sz = ImGui::GetContentRegionAvail();
				luexp(scene_renderer->init(scene_sz));
			}

			// Fetch camera and transform component.
			auto camera_entity = s->find_entity(scene_renderer->camera_entity);
			if (!camera_entity)
			{
				ImGui::Text("Camera Entity is not set in Scene Renderer Component.");
				return ok;
			}

			auto camera_component = camera_entity->get_component<Camera>();

			if (!camera_component)
			{
				ImGui::Text("Transform and Camera Component must be set to the Camera Entity set in Scene Renderer Component.");
				return ok;
			}

			ImGui::BeginChild("Scene Viewport", Float2(0.0f, 0.0f), false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

			ImGui::SetNextItemWidth(100.0f);
			ImGui::SliderFloat("Camera Speed", &m_camera_speed, 0.1f, 10.0f, "%.3f", 3.3f);
			ImGui::SameLine();
			{
				// Draw gizmo mode combo.
				ImGui::Text("Gizmo Mode");
				ImGui::SameLine();
				auto mode = m_gizmo_mode;
				if (m_gizmo_mode != ImGui::GizmoMode::local)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, Float4(1.0f, 1.0f, 1.0f, 0.5f));
				}
				if (ImGui::Button("Local"))
				{
					mode = ImGui::GizmoMode::local;
				}
				if (m_gizmo_mode != ImGui::GizmoMode::local)
				{
					ImGui::PopStyleColor();
				}
				ImGui::SameLine(0);
				if (m_gizmo_mode != ImGui::GizmoMode::world)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, Float4(1.0f, 1.0f, 1.0f, 0.5f));
				}
				if (ImGui::Button("World"))
				{
					mode = ImGui::GizmoMode::world;
				}
				if (m_gizmo_mode != ImGui::GizmoMode::world)
				{
					ImGui::PopStyleColor();
				}
				m_gizmo_mode = mode;
				ImGui::SameLine();

				// Draw gizmo operation combo.
				ImGui::Text("Gizmo Operation");
				ImGui::SameLine();
				auto op = m_gizmo_op;
				if (m_gizmo_op != ImGui::GizmoOperation::translate)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, Float4(1.0f, 1.0f, 1.0f, 0.5f));
				}
				if (ImGui::Button("Translate"))
				{
					op = ImGui::GizmoOperation::translate;
				}
				if (m_gizmo_op != ImGui::GizmoOperation::translate)
				{
					ImGui::PopStyleColor();
				}
				ImGui::SameLine(0);
				if (m_gizmo_op != ImGui::GizmoOperation::rotate)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, Float4(1.0f, 1.0f, 1.0f, 0.5f));
				}
				if (ImGui::Button("Rotate"))
				{
					op = ImGui::GizmoOperation::rotate;
				}
				if (m_gizmo_op != ImGui::GizmoOperation::rotate)
				{
					ImGui::PopStyleColor();
				}
				ImGui::SameLine(0);
				if (m_gizmo_op != ImGui::GizmoOperation::scale)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, Float4(1.0f, 1.0f, 1.0f, 0.5f));
				}
				if (ImGui::Button("Scale"))
				{
					op = ImGui::GizmoOperation::scale;
				}
				if (m_gizmo_op != ImGui::GizmoOperation::scale)
				{
					ImGui::PopStyleColor();
				}
				m_gizmo_op = op;
			}

			ImGui::SameLine();
			ImGui::Checkbox("Wireframe", &m_wireframe);
			ImGui::SameLine();
			ImGui::Checkbox("Grid", &m_grid);

			Float2 scene_sz = ImGui::GetContentRegionAvail();
			Float2 scene_pos = ImGui::GetCursorScreenPos();
			scene_sz.x -= 1.0f;
			scene_sz.y -= 5.0f;

			auto render_desc = scene_renderer->screen_buffer->get_desc();
			if (render_desc.width_or_buffer_size != scene_sz.x || render_desc.height != scene_sz.y)
			{
				luexp(scene_renderer->resize_screen_buffer(UInt2U((u32)scene_sz.x, (u32)scene_sz.y)));
			}

			auto render_tex = scene_renderer->screen_buffer;
			auto render_rtv = scene_renderer->screen_buffer_rtv;
			auto depth_tex = scene_renderer->depth_buffer;
			auto depth_dsv = scene_renderer->depth_buffer_dsv;

			render_desc = render_tex->get_desc();
			camera_component->aspect_ratio = (f32)render_desc.width_or_buffer_size / (f32)render_desc.height;

			// Update and upload camera data.
			m_camera_cb_data.world_to_view = camera_entity->world_to_local_matrix();
			m_camera_cb_data.view_to_proj = camera_component->get_projection_matrix();
			m_camera_cb_data.world_to_proj = mul(m_camera_cb_data.world_to_view, m_camera_cb_data.view_to_proj);
			m_camera_cb_data.view_to_world = camera_entity->local_to_world_matrix();
			Float3 env_color = scene_renderer->environment_color;
			m_camera_cb_data.env_color = Float4(env_color.x, env_color.y, env_color.z, 1.0f);
			void* mapped = nullptr;
			luexp(m_camera_cb->map_subresource(0, false, &mapped));
			memcpy(mapped, &m_camera_cb_data, sizeof(CameraCB));
			m_camera_cb->unmap_subresource(0, true);

			auto device = RHI::get_main_device();

			// Draw Scene.
			{
				using namespace RHI;

				// Clear render and stencil pass.
				{
					ResourceBarrierDesc ts[2] = {
						ResourceBarrierDesc::as_transition(render_tex, ResourceState::render_target),
						ResourceBarrierDesc::as_transition(depth_tex, ResourceState::depth_stencil_write)
					};
					m_scene_cmdbuf->resource_barriers({ ts, 2 });
					//m_back_buffer_clear_fbo = device->new_frame_buffer(m_back_buffer_clear_pass, 1, &render_tex, nullptr, depth_tex, nullptr).get();
					RenderPassDesc render_pass;
					render_pass.rtvs[0] = render_rtv;
					render_pass.dsv = depth_dsv;
					render_pass.rt_load_ops[0] = LoadOp::clear;
					render_pass.rt_clear_values[0] = Float4U(0.0f, 0.0f, 0.0f, 1.0f);
					render_pass.depth_load_op = LoadOp::clear;
					render_pass.depth_clear_value = 1.0f;
					m_scene_cmdbuf->begin_render_pass(render_pass);
					m_scene_cmdbuf->end_render_pass();
				}

				// Fetch meshes to draw.
				Vector<Ref<Entity>> ts;
				Vector<Ref<ModelRenderer>> rs;
				auto& entities = s->root_entities;
				for (auto& i : entities)
				{
					auto r = i->get_component<ModelRenderer>();
					if (r)
					{
						auto model = Asset::get_asset_data<Model>(r->model);
						if (!model)
						{
							continue;
						}
						auto mesh = Asset::get_asset_data<Mesh>(model->mesh);
						if (!mesh)
						{
							continue;
						}
						ts.push_back(i);
						rs.push_back(r);
					}
				}

				// Upload mesh matrices.
				{
					if (m_num_model_matrices < ts.size())
					{
						m_model_matrices = device->new_resource(ResourceDesc::buffer(ResourceHeapType::upload, ResourceUsageFlag::shader_resource, (u64)sizeof(Float4x4) * 2 * (u64)ts.size())).get();
						m_num_model_matrices = ts.size();
					}
					if (!ts.empty())
					{
						void* mapped = nullptr;
						luexp(m_model_matrices->map_subresource(0, false, &mapped));
						for (usize i = 0; i < ts.size(); ++i)
						{
							Float4x4 m2w = ts[i]->local_to_world_matrix();
							Float4x4 w2m = ts[i]->world_to_local_matrix();
							memcpy((Float4x4*)mapped + i * 2, m2w.r[0].m, sizeof(Float4x4));
							memcpy((Float4x4*)mapped + (i * 2 + 1), w2m.r[0].m, sizeof(Float4x4));
						}
						m_model_matrices->unmap_subresource(0, true);
					}
				}

				// Fetches lights to draw.
				Vector<Ref<Entity>> light_ts;
				Vector<ObjRef> light_rs;
				for (auto& i : entities)
				{
					ObjRef r = ObjRef(i->get_component<DirectionalLight>());
					if (!r)
					{
						r = i->get_component<PointLight>();
						if (!r)
						{
							r = i->get_component<SpotLight>();
						}
					}
					if (r)
					{
						light_ts.push_back(i);
						light_rs.push_back(r);
					}
				}

				// Upload lighting params.
				{
					if (m_num_lights < light_ts.size())
					{
						m_lighting_params = device->new_resource(ResourceDesc::buffer(ResourceHeapType::upload, ResourceUsageFlag::shader_resource, sizeof(LightingParams) * light_ts.size())).get();
						m_num_lights = light_ts.size();
					}
					void* mapped = nullptr;
					luexp(m_lighting_params->map_subresource(0, false, &mapped));
					for (usize i = 0; i < light_ts.size(); ++i)
					{
						LightingParams p;
						Ref<DirectionalLight> directional = light_rs[i];
						if (directional)
						{
							p.strength = directional->intensity;
							p.attenuation_power = 1.0f;
							p.direction = AffineMatrix3D::forward(AffineMatrix3D::make_rotation(light_ts[i]->world_rotation()));
							p.type = 0;
							p.position = light_ts[i]->world_position();
							p.spot_attenuation_power = 0.0f;
						}
						else
						{
							Ref<PointLight> point = light_rs[i];
							if (point)
							{
								p.strength = point->intensity;
								p.attenuation_power = point->attenuation_power;
								p.direction = Float3U(0.0f, 0.0f, 1.0f);
								p.type = 1;
								p.position = light_ts[i]->world_position();
								p.spot_attenuation_power = 0.0f;
							}
							else
							{
								Ref<SpotLight> spot = light_rs[i];
								if (spot)
								{
									p.strength = spot->intensity;
									p.attenuation_power = spot->attenuation_power;
									p.direction = AffineMatrix3D::forward(AffineMatrix3D::make_rotation(light_ts[i]->world_rotation()));
									p.type = 2;
									p.position = light_ts[i]->world_position();
									p.spot_attenuation_power = spot->spot_power;
								}
								else
								{
									lupanic_always();
								}
							}
						}
						memcpy((LightingParams*)mapped + i, &p, sizeof(LightingParams));
					}
					// Adds one fake light if there is no light so the SRV is not empty (which is invalid).
					if (light_ts.empty())
					{
						LightingParams p;
						p.strength = { 0.0f, 0.0f, 0.0f };
						p.attenuation_power = 1.0f;
						p.direction = { 0.0f, 0.0f, 1.0f };
						p.type = 0;
						p.position = { 0.0f, 0.0f, 0.0f };
						p.spot_attenuation_power = 0.0f;
						memcpy((LightingParams*)mapped, &p, sizeof(LightingParams));
					}
					m_lighting_params->unmap_subresource(0, true);
				}

				u32 cb_align = device->get_constant_buffer_data_alignment();

				if (m_wireframe)
				{
					// Debug wireframe pass.
					//auto debug_renderer_fbo = device->new_frame_buffer(m_type->m_debug_mesh_renderer_rp, 1, &render_tex, nullptr, nullptr, nullptr).get();
					RenderPassDesc render_pass;
					render_pass.rtvs[0] = render_rtv;
					
					m_scene_cmdbuf->begin_render_pass(render_pass);
					m_scene_cmdbuf->set_graphic_shader_input_layout(m_type->m_debug_mesh_renderer_slayout);
					m_scene_cmdbuf->set_pipeline_state(m_type->m_debug_mesh_renderer_pso);
					m_scene_cmdbuf->set_primitive_topology(PrimitiveTopology::triangle_list);
					m_scene_cmdbuf->set_viewport(Viewport(0.0f, 0.0f, (f32)render_desc.width_or_buffer_size, (f32)render_desc.height, 0.0f, 1.0f));
					m_scene_cmdbuf->set_scissor_rect(RectI(0, 0, (i32)render_desc.width_or_buffer_size, (i32)render_desc.height));
					// Draw Meshes.
					for (usize i = 0; i < ts.size(); ++i)
					{
						auto vs = device->new_descriptor_set(DescriptorSetDesc(m_type->m_debug_mesh_renderer_dlayout)).get();
						vs->set_cbv(0, m_camera_cb, ConstantBufferViewDesc(0, (u32)align_upper(sizeof(CameraCB), cb_align)));
						vs->set_srv(1, m_model_matrices, &ShaderResourceViewDesc::as_buffer(i, 1, sizeof(Float4x4) * 2, false));
						m_scene_cmdbuf->set_graphic_descriptor_set(0, vs);
						m_scene_cmdbuf->attach_graphic_object(vs);

						// Draw pieces.
						auto mesh = Asset::get_asset_data<Mesh>(Asset::get_asset_data<Model>(rs[i]->model)->mesh);

						auto vb_view = VertexBufferViewDesc(mesh->vb, 0,
							mesh->vb_count * sizeof(Vertex), sizeof(Vertex));

						m_scene_cmdbuf->set_vertex_buffers(0, { &vb_view, 1 });
						m_scene_cmdbuf->set_index_buffer(mesh->ib, 0, mesh->ib_count * sizeof(u32), Format::r32_uint);

						u32 num_pieces = (u32)mesh->pieces.size();
						for (u32 j = 0; j < num_pieces; ++j)
						{
							m_scene_cmdbuf->draw_indexed(mesh->pieces[j].num_indices, mesh->pieces[j].first_index_offset, 0);
						}
					}
					m_scene_cmdbuf->end_render_pass();
				}
				else
				{
					// Depth pass.
					{
						//auto fbo = device->new_frame_buffer(m_type->m_depth_pass_rp, 0, nullptr, nullptr, depth_tex, nullptr).get();
						
						//m_scene_cmdbuf->begin_render_pass(m_type->m_depth_pass_rp, fbo, 0, nullptr, 1.0f, 0xFF);
						//m_scene_cmdbuf->attach_graphic_object(fbo);
						//m_scene_cmdbuf->set_graphic_shader_input_layout(m_type->m_depth_pass_slayout);
						//m_scene_cmdbuf->set_pipeline_state(m_type->m_depth_pass_pso);
						//m_scene_cmdbuf->set_primitive_topology(PrimitiveTopology::triangle_list);
						//m_scene_cmdbuf->set_viewport(Viewport(0.0f, 0.0f, render_desc.width, render_desc.height, 0.0f, 1.0f));
						//m_scene_cmdbuf->set_scissor_rect(RectI(0, 0, render_desc.width, render_desc.height));
						//// Draw Meshes.
						//for (size_t i = 0; i < ts.size(); ++i)
						//{
						//	auto vs = device->new_descriptor_set(m_type->m_depth_pass_slayout, ViewSetDesc(1, 1, 0, 0)).get();
						//	vs->set_cbv(0, m_camera_cb, ConstantBufferViewDesc(0, align_upper(sizeof(CameraCB), cb_align)));
						//	vs->set_srv(0, m_model_matrices, &ShaderResourceViewDesc::as_buffer(Format::unknown, i, 1, sizeof(Float4x4), false));
						//	m_scene_cmdbuf->set_graphic_descriptor_set(vs);
						//	m_scene_cmdbuf->attach_graphic_object(vs);

						//	// Draw pieces.
						//	auto mesh = rs[i]->model().lock()->mesh().lock();
						//	m_scene_cmdbuf->set_vertex_buffers(0, 1, &VertexBufferViewDesc(mesh->vertex_buffer(), 0,
						//		mesh->count_vertices() * sizeof(Vertex), sizeof(Vertex)));
						//	m_scene_cmdbuf->set_index_buffer(mesh->index_buffer(), 0, mesh->count_indices() * sizeof(u32), Format::r32_uint);

						//	size_t num_pieces = mesh->count_pieces();
						//	for (size_t j = 0; j < num_pieces; ++j)
						//	{
						//		m_scene_cmdbuf->draw_indexed(mesh->piece_count_indices(j), mesh->piece_first_index_offset(j), 0);
						//	}
						//}
						//m_scene_cmdbuf->end_render_pass();
					}

					// Sky Box Pass.
					// Clears the lighting buffer to a skybox or black of the skybox is not present.
					{
						auto skybox = Asset::get_asset_data<RHI::IResource>(scene_renderer->skybox);
						if (skybox && camera_component->type == CameraType::perspective)
						{
							// Draw skybox.
							auto view_to_world = camera_entity->local_to_world_matrix();
							SkyboxParams* mapped = nullptr;
							luexp(m_skybox_params->map_subresource(0, false, (void**)&mapped));
							auto camera_forward_dir = mul(Float4(0.0f, 0.0f, 1.0f, 0.0f), camera_entity->local_to_world_matrix());
							memcpy(&mapped->view_to_world, &view_to_world, sizeof(Float4x4));
							mapped->fov = camera_component->fov;
							mapped->width = (u32)scene_sz.x;
							mapped->height = (u32)scene_sz.y;
							m_skybox_params->unmap_subresource(0, true);
							m_scene_cmdbuf->resource_barriers({
								ResourceBarrierDesc::as_transition(scene_renderer->lighting_buffer, ResourceState::unordered_access),
								ResourceBarrierDesc::as_transition(skybox, ResourceState::shader_resource_non_pixel),
								ResourceBarrierDesc::as_transition(m_skybox_params, ResourceState::vertex_and_constant_buffer)
								});
							m_scene_cmdbuf->set_compute_shader_input_layout(m_type->m_skybox_pass_slayout);
							m_scene_cmdbuf->set_pipeline_state(m_type->m_skybox_pass_pso);
							lulet(vs, device->new_descriptor_set(DescriptorSetDesc(m_type->m_skybox_pass_dlayout)));
							vs->set_cbv(0, m_skybox_params, ConstantBufferViewDesc(0, (u32)align_upper(sizeof(SkyboxParams), cb_align)));
							vs->set_srv(1, skybox);
							vs->set_uav(2, scene_renderer->lighting_buffer);
							vs->set_sampler(3, SamplerDesc(FilterMode::min_mag_mip_linear, TextureAddressMode::repeat, TextureAddressMode::repeat, TextureAddressMode::repeat));
							m_scene_cmdbuf->set_compute_descriptor_set(0, vs);
							m_scene_cmdbuf->attach_graphic_object(vs);
							m_scene_cmdbuf->dispatch(max<u32>((u32)scene_sz.x / 8, 1), max<u32>((u32)scene_sz.y / 8, 1), 1);
						}
						else
						{
							// Clears to black.
							m_scene_cmdbuf->resource_barrier(ResourceBarrierDesc::as_transition(scene_renderer->lighting_buffer, ResourceState::render_target));
							auto lighting_rt = scene_renderer->lighting_buffer;

							RenderPassDesc render_pass;
							render_pass.rtvs[0] = scene_renderer->lighting_buffer_rtv;
							render_pass.rt_load_ops[0] = LoadOp::clear;
							render_pass.rt_store_ops[0] = StoreOp::store;
							render_pass.rt_clear_values[0] = { 0.0f, 0.0f, 0.0f, 0.0f };
							m_scene_cmdbuf->begin_render_pass(render_pass);
							m_scene_cmdbuf->end_render_pass();
						}
					}

					// Lighting Pass.
					{
						//auto fbo = device->new_frame_buffer(m_type->m_lighting_pass_rp, 1, &lighting_rt, nullptr, depth_tex, nullptr).get();
						m_scene_cmdbuf->resource_barriers({ 
							ResourceBarrierDesc::as_transition(scene_renderer->lighting_buffer, ResourceState::render_target),
							ResourceBarrierDesc::as_transition(depth_tex, ResourceState::depth_stencil_write) });
						RenderPassDesc render_pass;
						render_pass.rtvs[0] = scene_renderer->lighting_buffer_rtv;
						render_pass.dsv = depth_dsv;
						m_scene_cmdbuf->begin_render_pass(render_pass);
						m_scene_cmdbuf->set_graphic_shader_input_layout(m_type->m_lighting_pass_slayout);
						m_scene_cmdbuf->set_pipeline_state(m_type->m_lighting_pass_pso);
						m_scene_cmdbuf->set_primitive_topology(PrimitiveTopology::triangle_list);
						m_scene_cmdbuf->set_viewport(Viewport(0.0f, 0.0f, (f32)render_desc.width_or_buffer_size, (f32)render_desc.height, 0.0f, 1.0f));
						m_scene_cmdbuf->set_scissor_rect(RectI(0, 0, (i32)render_desc.width_or_buffer_size, (i32)render_desc.height));

						// Draw Meshes.
						for (usize i = 0; i < ts.size(); ++i)
						{
							auto model = Asset::get_asset_data<Model>(rs[i]->model);
							auto mesh = Asset::get_asset_data<Mesh>(model->mesh);
							m_scene_cmdbuf->set_vertex_buffers(0, { VertexBufferViewDesc(mesh->vb, 0,
								mesh->vb_count * sizeof(Vertex), sizeof(Vertex)) });
							m_scene_cmdbuf->set_index_buffer(mesh->ib, 0, mesh->ib_count * sizeof(u32), Format::r32_uint);

							u32 num_pieces = (u32)mesh->pieces.size();

							for (u32 j = 0; j < num_pieces; ++j)
							{
								Ref<RHI::IResource> base_color_tex = m_type->m_default_base_color;
								Ref<RHI::IResource> roughness_tex = m_type->m_default_roughness;
								Ref<RHI::IResource> normal_tex = m_type->m_default_normal;
								Ref<RHI::IResource> metallic_tex = m_type->m_default_metallic;
								Ref<RHI::IResource> emissive_tex = m_type->m_default_emissive;
								Ref<RHI::IResource> sky_tex = m_type->m_default_emissive;

								if (j < model->materials.size())
								{
									auto mat = Asset::get_asset_data<Material>(model->materials[j]);
									if (mat)
									{
										// Set material for this piece.
										Ref<RHI::IResource> mat_base_color_tex = Asset::get_asset_data<RHI::IResource>(mat->base_color);
										Ref<RHI::IResource> mat_roughness_tex = Asset::get_asset_data<RHI::IResource>(mat->roughness);
										Ref<RHI::IResource> mat_normal_tex = Asset::get_asset_data<RHI::IResource>(mat->normal);
										Ref<RHI::IResource> mat_metallic_tex = Asset::get_asset_data<RHI::IResource>(mat->metallic);
										Ref<RHI::IResource> mat_emissive_tex = Asset::get_asset_data<RHI::IResource>(mat->emissive);
										if (mat_base_color_tex)
										{
											base_color_tex = mat_base_color_tex;
										}
										if (mat_roughness_tex)
										{
											roughness_tex = mat_roughness_tex;
										}
										if (mat_normal_tex)
										{
											normal_tex = mat_normal_tex;
										}
										if (mat_metallic_tex)
										{
											metallic_tex = mat_metallic_tex;
										}
										if (mat_emissive_tex)
										{
											emissive_tex = mat_emissive_tex;
										}
									}
								}

								auto skybox = Asset::get_asset_data<RHI::IResource>(scene_renderer->skybox);
								if (skybox) sky_tex = skybox;
								lulet(vs, device->new_descriptor_set(DescriptorSetDesc(m_type->m_lighting_pass_dlayout)));
								vs->set_cbv(0, m_camera_cb, ConstantBufferViewDesc(0, (u32)align_upper(sizeof(CameraCB), cb_align)));
								vs->set_srv(1, m_model_matrices, &ShaderResourceViewDesc::as_buffer(i, 1, sizeof(Float4x4) * 2, false));
								if (light_ts.empty())
								{
									// Adds one fake light.
									vs->set_srv(2, m_lighting_params, &ShaderResourceViewDesc::as_buffer(0, 1, sizeof(LightingParams)));
								}
								else
								{
									vs->set_srv(2, m_lighting_params, &ShaderResourceViewDesc::as_buffer(0, (u32)light_ts.size(), sizeof(LightingParams)));
								}
								// Set material texture: base_color(t2), roughness(t3), normal(t4), metallic(t5), emissive(t6).
								vs->set_srv(3, base_color_tex);
								vs->set_srv(4, roughness_tex);
								vs->set_srv(5, normal_tex);
								vs->set_srv(6, metallic_tex);
								vs->set_srv(7, emissive_tex);
								vs->set_srv(8, sky_tex);
								vs->set_sampler(9, SamplerDesc(FilterMode::min_mag_mip_linear, TextureAddressMode::repeat, TextureAddressMode::repeat, TextureAddressMode::repeat));
								m_scene_cmdbuf->set_graphic_descriptor_set(0, vs);
								m_scene_cmdbuf->attach_graphic_object(vs);
								m_scene_cmdbuf->draw_indexed(mesh->pieces[j].num_indices, mesh->pieces[j].first_index_offset, 0);
							}
						}
						m_scene_cmdbuf->end_render_pass();
					}

					// Bloom Pass.
					// The Bloom Pass is added to the lighting texture before it is tone-mapped.
					// The original light texture will also be scaled down a little bit.
					{

					}

					// Tone mapping pass.
					{
						// First Lum Pass.
						{
							m_scene_cmdbuf->set_compute_shader_input_layout(m_type->m_first_lum_pass_slayout);
							m_scene_cmdbuf->set_pipeline_state(m_type->m_first_lum_pass_pso);
							Float4 offsets[16];
							{
								// How much texels are covered by one sample pixel?
								f32 TexelsCoveredPerSampleW = scene_sz.x / 1024.0f;
								f32 TexelsCoveredPerSampleH = scene_sz.y / 1024.0f;
								// The offset of one texel in uv-space.
								f32 NormalizedWidthPerTexel = 1.0f / scene_sz.x;
								f32 NormalizedHeightPerTexel = 1.0f / scene_sz.y;
								for (i32 i = 0; i < 4; i++)
								{
									for (i32 j = 0; j < 4; j++)
									{
										offsets[4 * i + j] = Float4{
											NormalizedWidthPerTexel * TexelsCoveredPerSampleW / 8.0f * (float)(2 * j - 3) ,
											NormalizedHeightPerTexel * TexelsCoveredPerSampleH / 8.0f * (float)(2 * i - 3) ,0.0f,0.0f };
									}
								}
							}
							void* mapped = nullptr;
							luexp(m_tone_mapping_offset->map_subresource(0, false, &mapped));
							memcpy(mapped, offsets, sizeof(Float4) * 16);
							m_tone_mapping_offset->unmap_subresource(0, true);
							m_scene_cmdbuf->resource_barriers({ 
								ResourceBarrierDesc::as_transition(scene_renderer->lighting_buffer, ResourceState::shader_resource_non_pixel, 0),
								ResourceBarrierDesc::as_transition(scene_renderer->lighting_accms[0], ResourceState::unordered_access, 0),
								ResourceBarrierDesc::as_transition(m_tone_mapping_offset, ResourceState::vertex_and_constant_buffer, 0) });
							lulet(vs, device->new_descriptor_set(DescriptorSetDesc(m_type->m_first_lum_pass_dlayout)));
							vs->set_cbv(0, m_tone_mapping_offset, ConstantBufferViewDesc(0, (u32)align_upper(sizeof(Float4) * 16, cb_align)));
							vs->set_srv(1, scene_renderer->lighting_buffer);
							vs->set_uav(2, scene_renderer->lighting_accms[0]);
							vs->set_sampler(3, SamplerDesc(FilterMode::min_mag_mip_linear, TextureAddressMode::repeat, TextureAddressMode::repeat, TextureAddressMode::repeat));
							m_scene_cmdbuf->set_compute_descriptor_set(0, vs);
							m_scene_cmdbuf->attach_graphic_object(vs);
							m_scene_cmdbuf->dispatch(128, 128, 1);
						}

						// Lum passes.
						{
							m_scene_cmdbuf->set_compute_shader_input_layout(m_type->m_lum_pass_slayout);
							m_scene_cmdbuf->set_pipeline_state(m_type->m_lum_pass_pso);
							for (u32 i = 0; i < 10; ++i)
							{
								m_scene_cmdbuf->resource_barriers({
									ResourceBarrierDesc::as_transition(scene_renderer->lighting_accms[i], ResourceState::shader_resource_non_pixel, 0),
									ResourceBarrierDesc::as_transition(scene_renderer->lighting_accms[i + 1], ResourceState::unordered_access, 0) });
								lulet(vs, device->new_descriptor_set(DescriptorSetDesc(m_type->m_lum_pass_dlayout)));
								vs->set_srv(0, scene_renderer->lighting_accms[i]);
								vs->set_uav(1, scene_renderer->lighting_accms[i + 1]);
								m_scene_cmdbuf->set_compute_descriptor_set(0, vs);
								m_scene_cmdbuf->attach_graphic_object(vs);
								u32 dispatches = max(64 >> i, 1);
								m_scene_cmdbuf->dispatch(dispatches, dispatches, 1);
							}
						}

						// Tone Mapping Pass.
						{
							void* mapped = nullptr;
							luexp(m_tone_mapping_params->map_subresource(0, false, &mapped));
							ToneMappingParams params;
							params.exposure = scene_renderer->exposure;
							memcpy(mapped, &params, sizeof(ToneMappingParams));
							m_tone_mapping_params->unmap_subresource(0, true);
							m_scene_cmdbuf->set_compute_shader_input_layout(m_type->m_tone_mapping_pass_slayout);
							m_scene_cmdbuf->set_pipeline_state(m_type->m_tone_mapping_pass_pso);
							m_scene_cmdbuf->resource_barriers({
								ResourceBarrierDesc::as_transition(scene_renderer->lighting_accms[10], ResourceState::shader_resource_non_pixel),
								ResourceBarrierDesc::as_transition(scene_renderer->lighting_buffer, ResourceState::shader_resource_non_pixel),
								ResourceBarrierDesc::as_transition(scene_renderer->screen_buffer, ResourceState::unordered_access),
								ResourceBarrierDesc::as_transition(m_tone_mapping_params, ResourceState::vertex_and_constant_buffer) });
							lulet(vs, device->new_descriptor_set(DescriptorSetDesc(m_type->m_tone_mapping_pass_dlayout)));
							vs->set_cbv(0, m_tone_mapping_params, ConstantBufferViewDesc(0, (u32)align_upper(sizeof(ToneMappingParams), cb_align)));
							vs->set_srv(1, scene_renderer->lighting_buffer);
							vs->set_srv(2, scene_renderer->lighting_accms[10]);
							vs->set_uav(3, scene_renderer->screen_buffer);
							m_scene_cmdbuf->set_compute_descriptor_set(0, vs);
							m_scene_cmdbuf->attach_graphic_object(vs);
							m_scene_cmdbuf->dispatch(max<u32>((u32)scene_sz.x / 8, 1), max<u32>((u32)scene_sz.y / 8, 1), 1);
						}
					}

				}
			}

			// Draw Overlays.
			if(m_grid)
			{
				// Draw Grid.
				using namespace RHI;
				//m_grid_fb = device->new_frame_buffer(m_type->m_grid_rp, 1, &render_tex, nullptr, nullptr, nullptr).get();
				m_scene_cmdbuf->resource_barrier(ResourceBarrierDesc::as_transition(render_tex, ResourceState::render_target));
				RenderPassDesc render_pass;
				render_pass.rtvs[0] = render_rtv;
				m_scene_cmdbuf->begin_render_pass(render_pass);
				m_scene_cmdbuf->set_graphic_shader_input_layout(m_type->m_grid_slayout);
				m_scene_cmdbuf->set_pipeline_state(m_type->m_grid_pso);
				m_scene_cmdbuf->set_vertex_buffers(0, { VertexBufferViewDesc(m_type->m_grid_vb.get(), 0, sizeof(Float4) * 44, sizeof(Float4)) });
				m_scene_cmdbuf->set_primitive_topology(PrimitiveTopology::line_list);
				m_scene_cmdbuf->set_graphic_descriptor_set(0, m_grid_desc_set);
				m_scene_cmdbuf->set_viewport(Viewport(0.0f, 0.0f, (f32)render_desc.width_or_buffer_size, (f32)render_desc.height, 0.0f, 1.0f));
				m_scene_cmdbuf->set_scissor_rect(RectI(0, 0, (i32)render_desc.width_or_buffer_size, (i32)render_desc.height));
				m_scene_cmdbuf->draw(44, 0);
				m_scene_cmdbuf->end_render_pass();
			}

			luexp(m_scene_cmdbuf->submit());

			ImGui::Image(render_tex, scene_sz);

			// Draw GUI Overlays.
			{
				// Draw gizmo.
				auto e = m_current_entity.pin();
				if (e && e != camera_entity)
				{
					Float4x4 world_mat = e->local_to_world_matrix();
					bool edited = false;
					ImGui::Gizmo(world_mat, m_camera_cb_data.world_to_view, m_camera_cb_data.view_to_proj,
						RectF(scene_pos.x, scene_pos.y, scene_sz.x, scene_sz.y), m_gizmo_op, m_gizmo_mode, 0.0f, true, false, nullptr, nullptr, &edited);
					if (edited)
					{
						e->set_local_to_world_matrix(world_mat);
					}
				}

				// Draw scene debug info.
				auto backup_pos = ImGui::GetCursorPos();
				ImGui::SetCursorScreenPos(scene_pos);

				ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);


				ImGui::SetCursorPos(backup_pos);
			}

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && in_rect(ImGui::GetIO().MousePos, scene_pos, scene_pos + scene_sz))
			{
				m_navigating = true;
				m_scene_click_pos = HID::get_device<HID::IMouse>().get()->get_cursor_pos();
			}

			if (m_navigating && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
			{
				m_navigating = false;
			}

			if (m_navigating)
			{
				auto mouse = HID::get_device<HID::IMouse>().get();
				auto mouse_pos = mouse->get_cursor_pos();
				auto mouse_delta = mouse_pos - m_scene_click_pos;
				auto _ = mouse->set_cursor_pos(m_scene_click_pos.x, m_scene_click_pos.y);
				// Rotate camera based on mouse delta.
				auto rot = camera_entity->rotation;
				auto rot_mat = AffineMatrix3D::make_rotation(rot);

				// Key control.
				auto left = AffineMatrix3D::left(rot_mat);
				auto forward = AffineMatrix3D::forward(rot_mat);
				auto up = AffineMatrix3D::up(rot_mat);

				f32 camera_speed = m_camera_speed;
				auto& io = ImGui::GetIO();
				if (io.KeysDown[ImGuiKey_LeftShift])
				{
					camera_speed *= 2.0f;
				}

				if (io.KeysDown[ImGuiKey_W])
				{
					camera_entity->position += forward * 0.1f * camera_speed;
				}
				if (io.KeysDown[ImGuiKey_A])
				{
					camera_entity->position += + left * 0.1f * camera_speed;
				}
				if (io.KeysDown[ImGuiKey_S])
				{
					camera_entity->position += - forward * 0.1f * camera_speed;
				}
				if (io.KeysDown[ImGuiKey_D])
				{
					camera_entity->position += - left * 0.1f * camera_speed;
				}
				if (io.KeysDown[ImGuiKey_Q])
				{
					camera_entity->position += - up * 0.1f * camera_speed;
				}
				if (io.KeysDown[ImGuiKey_E])
				{
					camera_entity->position += + up * 0.1f * camera_speed;
				}
				auto eular = rot_mat.euler_angles();
				eular += {deg_to_rad((f32)mouse_delta.y / 10.0f), deg_to_rad((f32)mouse_delta.x / 10.0f), 0.0f};
				eular.x = clamp(eular.x, deg_to_rad(-85.0f), deg_to_rad(85.0f));
				camera_entity->rotation = Quaternion::from_euler_angles(eular);
			}


			m_scene_cmdbuf->wait();
			luassert_always(succeeded(m_scene_cmdbuf->reset()));

			ImGui::EndChild();
		}
		lucatchret;
		return ok;

		
	}

	static void draw_transform(Entity* t)
	{
		ImGui::DragFloat3("Position", t->position.m, 0.01f);

		auto euler = AffineMatrix3D::make_rotation(t->rotation).euler_angles();
		euler *= 180.0f / PI;
		if (euler.x > 89.0f || euler.x < -89.0f)
		{
			euler.z = 0.0f;
		}
		ImGui::DragFloat3("Rotation", euler.m);
		if (ImGui::IsItemEdited())
		{
			euler *= PI / 180.0f;
			t->rotation = Quaternion::from_euler_angles(euler);
		}

		ImGui::DragFloat3("Scale", t->scale.m, 0.01f);
	}

	void SceneEditor::draw_components_grid()
	{
		// Draw component property grid.

		ImGui::Text("Components Grid");

		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
		ImGui::BeginChild("Components Grid", Float2(0.0f, 0.0f), true);

		auto current_entity = m_current_entity.pin();

		if (current_entity)
		{
			// Draw transform.
			draw_transform(current_entity);

			if (current_entity->components.empty())
			{
				ImGui::Text("No components");
			}
			else
			{
				auto iter = current_entity->components.begin();

				while (iter != current_entity->components.end())
				{
					if (ImGui::CollapsingHeader(get_type_name(iter->first).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
					{
						edit_object(iter->second.get());
						ImGui::PushID(iter->first);

						if (ImGui::Button("Remove"))
						{
							iter = current_entity->components.erase(iter);
						}
						else
						{
							++iter;
						}

						ImGui::PopID();
					}
				}
			}

			const char* new_comp_popup = "NewCompPopup";

			if (ImGui::Button("New Component"))
			{
				ImGui::OpenPopup(new_comp_popup);
			}

			if (ImGui::BeginPopup(new_comp_popup))
			{

				for (auto& i : g_env->component_types)
				{
					auto name = get_type_name(i);
					auto exists = current_entity->components.contains(i);
					if (!exists)
					{
						// Show enabled.
						if (ImGui::Selectable(name.c_str()))
						{
							object_t comp = object_alloc(i);
							construct_type(i, comp);
							ObjRef comp_obj;
							comp_obj.attach(comp);
							current_entity->components.insert(make_pair(i, move(comp_obj)));
							ImGui::CloseCurrentPopup();
						}
					}
					else
					{
						// Show disabled.
						ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_Disabled);
					}
				}
				ImGui::EndPopup();
			}
		}
		else
		{
			ImGui::Text("Select an entity to see components.");
		}

		ImGui::EndChild();
		ImGui::PopStyleVar();
	}
	void SceneEditor::on_render()
	{
		char title[32];
		sprintf_s(title, "Scene Editor###%d", (u32)(usize)this);
		ImGui::SetNextWindowSize(Float2(1000, 500), ImGuiCond_FirstUseEver);
		ImGui::Begin(title, &m_open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);
		auto s = Asset::get_asset_data<Scene>(m_scene);
		if (!s)
		{
			ImGui::Text("Asset Unloaded");
			ImGui::End();
			return;
		}
		if (Asset::get_asset_state(m_scene) == Asset::AssetState::unloaded)
		{
			Asset::load_asset(m_scene);
		}
		if (Asset::get_asset_state(m_scene) != Asset::AssetState::loaded)
		{
			ImGui::Text("Scene Loading");
			ImGui::End();
			return;
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save"))
				{
					lutry
					{
						luexp(Asset::save_asset(m_scene));
					}
					lucatch
					{
						auto _ = Window::message_box(explain(lures), "Failed to save scene", Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
					}
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::Columns(3, nullptr, true);

		draw_entity_list();

		draw_scene_components_grid();

		ImGui::NextColumn();

		draw_scene();

		ImGui::NextColumn();

		draw_components_grid();

		ImGui::NextColumn();

		ImGui::End();
	}

	RV SceneEditorUserData::init()
	{
		//! Initialize Grid data.
		Float4 grids[44];
		for (i32 i = -5; i <= 5; ++i) // 0 - 21
		{
			grids[(i + 5) * 2] = Float4((f32)i, 0.0f, 5.0f, 1.0f);
			grids[(i + 5) * 2 + 1] = Float4((f32)i, 0.0f, -5.0f, 1.0f);
		}
		for (i32 i = -5; i <= 5; ++i) // 22 - 43
		{
			grids[(i + 5) * 2 + 22] = Float4(-5.0f, 0.0f, (f32)i, 1.0f);
			grids[(i + 5) * 2 + 23] = Float4(5.0f, 0.0f, (f32)i, 1.0f);
		}

		lutry
		{
			using namespace RHI;
			auto device = get_main_device();
			{
				luset(m_grid_vb, device->new_resource(ResourceDesc::buffer(ResourceHeapType::shared_upload, ResourceUsageFlag::vertex_buffer, sizeof(grids))));
				luset(m_default_base_color, device->new_resource(
					ResourceDesc::tex2d(ResourceHeapType::shared_upload, Format::rgba8_unorm, ResourceUsageFlag::shader_resource, 1, 1, 1, 1)));
				luset(m_default_roughness, device->new_resource(
					ResourceDesc::tex2d(ResourceHeapType::shared_upload, Format::r8_unorm, ResourceUsageFlag::shader_resource, 1, 1, 1, 1)));
				luset(m_default_normal, device->new_resource(
					ResourceDesc::tex2d(ResourceHeapType::shared_upload, Format::rgba8_unorm, ResourceUsageFlag::shader_resource, 1, 1, 1, 1)));
				luset(m_default_metallic, device->new_resource(
					ResourceDesc::tex2d(ResourceHeapType::shared_upload, Format::r8_unorm, ResourceUsageFlag::shader_resource, 1, 1, 1, 1)));
				luset(m_default_emissive, device->new_resource(
					ResourceDesc::tex2d(ResourceHeapType::shared_upload, Format::rgba8_unorm, ResourceUsageFlag::shader_resource, 1, 1, 1, 1)));

				DescriptorSetLayoutDesc dlayout({
					DescriptorSetLayoutBinding(DescriptorType::cbv, 0, 1, ShaderVisibility::vertex)
					});
				luset(m_grid_dlayout, device->new_descriptor_set_layout(dlayout));
				luset(m_grid_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ m_grid_dlayout },
					ShaderInputLayoutFlag::allow_input_assembler_input_layout |
					ShaderInputLayoutFlag::deny_domain_shader_access |
					ShaderInputLayoutFlag::deny_geometry_shader_access |
					ShaderInputLayoutFlag::deny_hull_shader_access)));
				static const char* vertexShader =
					"cbuffer vertexBuffer : register(b0) \
						{\
							float4x4 world_to_view; \
							float4x4 view_to_proj; \
							float4x4 world_to_proj; \
							float4x4 view_to_world; \
						};\
						struct VS_INPUT\
						{\
						  float4 pos : POSITION;\
						};\
						\
						struct PS_INPUT\
						{\
						  float4 pos : SV_POSITION;\
						};\
						\
						PS_INPUT main(VS_INPUT input)\
						{\
						  PS_INPUT output;\
						  output.pos = mul(world_to_proj, input.pos);\
						  return output;\
						}";
				auto compiler = ShaderCompiler::new_compiler();
				compiler->set_source({ vertexShader, strlen(vertexShader) });
				compiler->set_source_name("GridVS");
				compiler->set_entry_point("main");
				compiler->set_target_format(get_current_platform_shader_target_format());
				compiler->set_shader_type(ShaderCompiler::ShaderType::vertex);
				compiler->set_shader_model(5, 0);
				compiler->set_optimization_level(ShaderCompiler::OptimizationLevel::full);
				luexp(compiler->compile());
				Blob vs_blob;
				{
					auto output = compiler->get_output();
					vs_blob = Blob(output.data(), output.size());
				}
				static const char* pixelShader =
					"struct PS_INPUT\
					{\
						float4 pos : SV_POSITION;\
					};\
					\
					float4 main(PS_INPUT input) : SV_Target\
					{\
						return float4(1.0f, 1.0f, 1.0f, 1.0f); \
					}";

				compiler->reset();
				compiler->set_source({ pixelShader, strlen(pixelShader) });
				compiler->set_source_name("GridPS");
				compiler->set_entry_point("main");
				compiler->set_target_format(get_current_platform_shader_target_format());
				compiler->set_shader_type(ShaderCompiler::ShaderType::pixel);
				compiler->set_shader_model(5, 0);
				compiler->set_optimization_level(ShaderCompiler::OptimizationLevel::full);
				luexp(compiler->compile());
				Blob ps_blob = compiler->get_output();

				GraphicPipelineStateDesc ps_desc;
				ps_desc.primitive_topology_type = PrimitiveTopologyType::line;
				ps_desc.blend_state = BlendDesc(false, false, { RenderTargetBlendDesc(true, false, BlendFactor::src_alpha,
					BlendFactor::inv_src_alpha, BlendOp::add, BlendFactor::inv_src_alpha, BlendFactor::zero, BlendOp::add, LogicOp::noop, ColorWriteMask::all) });
				ps_desc.rasterizer_state = RasterizerDesc(FillMode::wireframe, CullMode::none, 0, 0.0f, 0.0f, 1, false, true, false, true, false);
				ps_desc.depth_stencil_state = DepthStencilDesc(false, false, ComparisonFunc::always, false, 0x00, 0x00, DepthStencilOpDesc(), DepthStencilOpDesc());
				ps_desc.ib_strip_cut_value = IndexBufferStripCutValue::disabled;
				ps_desc.input_layout.input_elements = {
					InputElementDesc("POSITION", 0, Format::rgba32_float)
				};
				ps_desc.shader_input_layout = m_grid_slayout;
				ps_desc.vs = { vs_blob.data(), vs_blob.size() };
				ps_desc.ps = { ps_blob.data(), ps_blob.size() };
				ps_desc.num_render_targets = 1;
				ps_desc.rtv_formats[0] = Format::rgba8_unorm;

				luset(m_grid_pso, device->new_graphic_pipeline_state(ps_desc));
			}

			// Upload grid vertex data.
			void* mapped = nullptr;
			luexp(m_grid_vb->map_subresource(0, false, &mapped));
			memcpy(mapped, grids, sizeof(grids));
			m_grid_vb->unmap_subresource(0, true);

			// Upload default texture data.
			luexp(m_default_base_color->map_subresource(0, false));
			luexp(m_default_roughness->map_subresource(0, false));
			luexp(m_default_normal->map_subresource(0, false));
			luexp(m_default_metallic->map_subresource(0, false));
			luexp(m_default_emissive->map_subresource(0, false));
			u8 data[4] = { 255, 255, 255, 255 };
			luexp(m_default_base_color->write_subresource(0, data, 4, 4, BoxU(0, 0, 0, 1, 1, 1)));
			data[0] = 127;
			luexp(m_default_roughness->write_subresource(0, data, 1, 1, BoxU(0, 0, 0, 1, 1, 1)));
			data[0] = 127;
			data[1] = 127;
			data[2] = 255;
			data[3] = 255;
			luexp(m_default_normal->write_subresource(0, data, 4, 4, BoxU(0, 0, 0, 1, 1, 1)));
			data[0] = 0;
			luexp(m_default_metallic->write_subresource(0, data, 1, 1, BoxU(0, 0, 0, 1, 1, 1)));
			data[0] = 0;
			data[1] = 0;
			data[2] = 0;
			data[3] = 0;
			luexp(m_default_emissive->write_subresource(0, data, 4, 4, BoxU(0, 0, 0, 1, 1, 1)));
			m_default_base_color->unmap_subresource(0, true);
			m_default_roughness->unmap_subresource(0, true);
			m_default_normal->unmap_subresource(0, true);
			m_default_metallic->unmap_subresource(0, true);
			m_default_emissive->unmap_subresource(0, true);

			static const char* vertexShaderCommon =
				"cbuffer vertexBuffer : register(b0) \
						{\
							float4x4 world_to_view; \
							float4x4 view_to_proj; \
							float4x4 world_to_proj; \
							float4x4 view_to_world; \
							float4 env_light_color; \
						};\
						struct MeshBuffer	\
						{\
							float4x4 model_to_world;	\
							float4x4 world_to_model;	\
						};\
						StructuredBuffer<MeshBuffer> g_MeshBuffer : register(t1);\
						struct VS_INPUT\
						{\
							float3 position : POSITION;	\
							float3 normal : NORMAL;	\
							float3 tangent : TANGENT;	\
							float2 texcoord : TEXCOORD;	\
							float4 color : COLOR;	\
						};\
						\
						struct PS_INPUT\
						{\
							float4 position : SV_POSITION;	\
							float3 normal : NORMAL;	\
							float3 tangent : TANGENT;	\
							float2 texcoord : TEXCOORD;	\
							float4 color : COLOR;	\
							float3 world_position : POSITION;	\
						};\
						\
						PS_INPUT main(VS_INPUT input)\
						{\
							PS_INPUT output;\
							output.world_position = mul(g_MeshBuffer[0].model_to_world, float4(input.position, 1.0f)).xyz;\
							output.position = mul(world_to_proj, float4(output.world_position, 1.0f));\
							output.normal = mul(float4(input.normal, 0.0f), g_MeshBuffer[0].world_to_model).xyz;\
							output.tangent = mul(float4(input.tangent, 0.0f), g_MeshBuffer[0].world_to_model).xyz;\
							output.texcoord = input.texcoord;	\
							output.color = input.color;	\
							return output;\
						}";

				auto compiler = ShaderCompiler::new_compiler();
				compiler->set_source({ vertexShaderCommon, strlen(vertexShaderCommon) });
				compiler->set_source_name("MeshDebugVS");
				compiler->set_entry_point("main");
				compiler->set_target_format(get_current_platform_shader_target_format());
				compiler->set_shader_type(ShaderCompiler::ShaderType::vertex);
				compiler->set_shader_model(5, 0);
				compiler->set_optimization_level(ShaderCompiler::OptimizationLevel::full);
				luexp(compiler->compile());

				Blob vs_blob = compiler->get_output();

				InputLayoutDesc input_layout_common({
						InputElementDesc("POSITION", 0, Format::rgb32_float),
						InputElementDesc("NORMAL", 0, Format::rgb32_float),
						InputElementDesc("TANGENT", 0, Format::rgb32_float),
						InputElementDesc("TEXCOORD", 0, Format::rg32_float),
						InputElementDesc("COLOR", 0, Format::rgba32_float),
					});

				// Create Resources for debug mesh renderer.
				{

					luset(m_debug_mesh_renderer_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
						DescriptorSetLayoutBinding(DescriptorType::cbv, 0, 1, ShaderVisibility::vertex),
						DescriptorSetLayoutBinding(DescriptorType::srv, 1, 1, ShaderVisibility::vertex) })));

					luset(m_debug_mesh_renderer_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ m_debug_mesh_renderer_dlayout },
						ShaderInputLayoutFlag::allow_input_assembler_input_layout |
						ShaderInputLayoutFlag::deny_domain_shader_access |
						ShaderInputLayoutFlag::deny_geometry_shader_access |
						ShaderInputLayoutFlag::deny_hull_shader_access)));
					static const char* pixelShader =
						"struct PS_INPUT\
						{\
							float4 position : SV_POSITION;	\
							float3 normal : NORMAL;	\
							float3 tangent : TANGENT;	\
							float2 texcoord : TEXCOORD;	\
							float4 color : COLOR;	\
							float3 world_position : POSITION;	\
						}; \
						\
						float4 main(PS_INPUT input) : SV_Target\
						{\
						  return float4(1.0f, 1.0f, 1.0f, 1.0f); \
						}";
					compiler->set_source({ pixelShader, strlen(pixelShader) });
					compiler->set_source_name("MeshDebugPS");
					compiler->set_entry_point("main");
					compiler->set_target_format(get_current_platform_shader_target_format());
					compiler->set_shader_type(ShaderCompiler::ShaderType::pixel);
					compiler->set_shader_model(5, 0);
					compiler->set_optimization_level(ShaderCompiler::OptimizationLevel::full);
					luexp(compiler->compile());
					Blob ps_blob = compiler->get_output();

					GraphicPipelineStateDesc ps_desc;
					ps_desc.primitive_topology_type = PrimitiveTopologyType::triangle;
					ps_desc.sample_mask = U32_MAX;
					ps_desc.sample_quality = 0;
					ps_desc.blend_state = BlendDesc(false, false, { RenderTargetBlendDesc(true, false, BlendFactor::src_alpha,
						BlendFactor::inv_src_alpha, BlendOp::add, BlendFactor::inv_src_alpha, BlendFactor::zero, BlendOp::add, LogicOp::noop, ColorWriteMask::all) });
					ps_desc.rasterizer_state = RasterizerDesc(FillMode::wireframe, CullMode::none, 0, 0.0f, 0.0f, 0, false, true, false, true, false);
					ps_desc.depth_stencil_state = DepthStencilDesc(false, false, ComparisonFunc::always, false, 0x00, 0x00, DepthStencilOpDesc(), DepthStencilOpDesc());
					ps_desc.ib_strip_cut_value = IndexBufferStripCutValue::disabled;
					ps_desc.input_layout = input_layout_common;
					ps_desc.vs = vs_blob.cspan();
					ps_desc.ps = ps_blob.cspan();
					ps_desc.shader_input_layout = m_debug_mesh_renderer_slayout;
					ps_desc.num_render_targets = 1;
					ps_desc.rtv_formats[0] = Format::rgba8_unorm;
					luset(m_debug_mesh_renderer_pso, device->new_graphic_pipeline_state(ps_desc));
				}

				// Depth Pass.
				{
					luset(m_depth_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
						DescriptorSetLayoutBinding(DescriptorType::cbv, 0, 1, ShaderVisibility::vertex),
						DescriptorSetLayoutBinding(DescriptorType::srv, 1, 1, ShaderVisibility::vertex)
						})));
					luset(m_depth_pass_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ m_depth_pass_dlayout },
						ShaderInputLayoutFlag::allow_input_assembler_input_layout |
						ShaderInputLayoutFlag::deny_domain_shader_access |
						ShaderInputLayoutFlag::deny_geometry_shader_access |
						ShaderInputLayoutFlag::deny_hull_shader_access |
						ShaderInputLayoutFlag::deny_pixel_shader_access)));

					GraphicPipelineStateDesc ps_desc;
					ps_desc.primitive_topology_type = PrimitiveTopologyType::triangle;
					ps_desc.sample_mask = U32_MAX;
					ps_desc.sample_quality = 0;
					ps_desc.blend_state = BlendDesc(false, false, {});
					ps_desc.rasterizer_state = RasterizerDesc(FillMode::solid, CullMode::back, 0, 0.0f, 0.0f, 0, false, true, false, false, false);
					ps_desc.depth_stencil_state = DepthStencilDesc(true, true, ComparisonFunc::less , false, 0x00, 0x00, DepthStencilOpDesc(), DepthStencilOpDesc());
					ps_desc.ib_strip_cut_value = IndexBufferStripCutValue::disabled;
					ps_desc.input_layout = input_layout_common;
					ps_desc.shader_input_layout = m_depth_pass_slayout;
					ps_desc.vs = vs_blob.cspan();
					ps_desc.dsv_format = Format::d32_float;

					luset(m_depth_pass_pso, device->new_graphic_pipeline_state(ps_desc));
				}

				// Skybox pass.
				{
					luset(m_skybox_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
						DescriptorSetLayoutBinding(DescriptorType::cbv, 0, 1, ShaderVisibility::all),
						DescriptorSetLayoutBinding(DescriptorType::srv, 1, 1, ShaderVisibility::all),
						DescriptorSetLayoutBinding(DescriptorType::uav, 2, 1, ShaderVisibility::all),
						DescriptorSetLayoutBinding(DescriptorType::sampler, 3, 1, ShaderVisibility::all)
						})));

					luset(m_skybox_pass_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ m_skybox_pass_dlayout },
						ShaderInputLayoutFlag::deny_vertex_shader_access |
						ShaderInputLayoutFlag::deny_domain_shader_access |
						ShaderInputLayoutFlag::deny_geometry_shader_access |
						ShaderInputLayoutFlag::deny_hull_shader_access |
						ShaderInputLayoutFlag::deny_pixel_shader_access)));

					lulet(psf, open_file("SkyboxCS.cso", FileOpenFlag::read, FileCreationMode::open_existing));
					auto file_size = psf->get_size();
					auto cs_blob = Blob((usize)file_size);
					luexp(psf->read(cs_blob.data(), cs_blob.size()));
					psf = nullptr;
					ComputePipelineStateDesc ps_desc;
					ps_desc.cs = cs_blob.cspan();
					ps_desc.shader_input_layout = m_skybox_pass_slayout;
					luset(m_skybox_pass_pso, device->new_compute_pipeline_state(ps_desc));
				}

				// Lighting Pass.
				{
					luset(m_lighting_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
						DescriptorSetLayoutBinding(DescriptorType::cbv, 0, 1, ShaderVisibility::all),
						DescriptorSetLayoutBinding(DescriptorType::srv, 1, 1, ShaderVisibility::all),
						DescriptorSetLayoutBinding(DescriptorType::srv, 2, 1, ShaderVisibility::pixel),
						DescriptorSetLayoutBinding(DescriptorType::srv, 3, 1, ShaderVisibility::pixel),
						DescriptorSetLayoutBinding(DescriptorType::srv, 4, 1, ShaderVisibility::pixel),
						DescriptorSetLayoutBinding(DescriptorType::srv, 5, 1, ShaderVisibility::pixel),
						DescriptorSetLayoutBinding(DescriptorType::srv, 6, 1, ShaderVisibility::pixel),
						DescriptorSetLayoutBinding(DescriptorType::srv, 7, 1, ShaderVisibility::pixel),
						DescriptorSetLayoutBinding(DescriptorType::srv, 8, 1, ShaderVisibility::pixel),
						DescriptorSetLayoutBinding(DescriptorType::sampler, 9, 1, ShaderVisibility::pixel)
						})));

					luset(m_lighting_pass_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ m_lighting_pass_dlayout },
						ShaderInputLayoutFlag::allow_input_assembler_input_layout |
						ShaderInputLayoutFlag::deny_domain_shader_access |
						ShaderInputLayoutFlag::deny_geometry_shader_access |
						ShaderInputLayoutFlag::deny_hull_shader_access)));

					lulet(psf, open_file("LightingPassPixel.cso", FileOpenFlag::read, FileCreationMode::open_existing));
					auto file_size = psf->get_size();
					auto ps_blob = Blob((usize)file_size);
					luexp(psf->read(ps_blob.data(), ps_blob.size()));
					psf = nullptr;

					GraphicPipelineStateDesc ps_desc;
					ps_desc.primitive_topology_type = PrimitiveTopologyType::triangle;
					ps_desc.sample_mask = U32_MAX;
					ps_desc.sample_quality = 0;
					ps_desc.blend_state = BlendDesc(false, false, { RenderTargetBlendDesc(false, false, BlendFactor::src_alpha,
						BlendFactor::inv_src_alpha, BlendOp::add, BlendFactor::inv_src_alpha, BlendFactor::zero, BlendOp::add, LogicOp::noop, ColorWriteMask::all) });
					ps_desc.rasterizer_state = RasterizerDesc(FillMode::solid, CullMode::back, 0, 0.0f, 0.0f, 0, false, true, false, false, false);
					ps_desc.depth_stencil_state = DepthStencilDesc(true, true, ComparisonFunc::less_equal, false, 0x00, 0x00, DepthStencilOpDesc(), DepthStencilOpDesc());
					ps_desc.ib_strip_cut_value = IndexBufferStripCutValue::disabled;
					ps_desc.input_layout = input_layout_common;
					ps_desc.vs = vs_blob.cspan();
					ps_desc.ps = ps_blob.cspan();
					ps_desc.shader_input_layout = m_lighting_pass_slayout;
					ps_desc.num_render_targets = 1;
					ps_desc.rtv_formats[0] = Format::rgba32_float;
					ps_desc.dsv_format = Format::d32_float;
					luset(m_lighting_pass_pso, device->new_graphic_pipeline_state(ps_desc));
				}

				//First Lum Pass.
				{
					luset(m_first_lum_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
						DescriptorSetLayoutBinding(DescriptorType::cbv, 0, 1, ShaderVisibility::all),
						DescriptorSetLayoutBinding(DescriptorType::srv, 1, 1, ShaderVisibility::all),
						DescriptorSetLayoutBinding(DescriptorType::uav, 2, 1, ShaderVisibility::all),
						DescriptorSetLayoutBinding(DescriptorType::sampler, 3, 1, ShaderVisibility::all)
						})));

					luset(m_first_lum_pass_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ m_first_lum_pass_dlayout },
						ShaderInputLayoutFlag::deny_vertex_shader_access |
						ShaderInputLayoutFlag::deny_domain_shader_access |
						ShaderInputLayoutFlag::deny_geometry_shader_access |
						ShaderInputLayoutFlag::deny_hull_shader_access |
						ShaderInputLayoutFlag::deny_pixel_shader_access)));

					lulet(psf, open_file("LumFirstCS.cso", FileOpenFlag::read, FileCreationMode::open_existing));
					auto file_size = psf->get_size();
					auto cs_blob = Blob((usize)file_size);
					luexp(psf->read(cs_blob.data(), cs_blob.size()));
					psf = nullptr;
					ComputePipelineStateDesc ps_desc;
					ps_desc.cs = cs_blob.cspan();
					ps_desc.shader_input_layout = m_first_lum_pass_slayout;
					luset(m_first_lum_pass_pso, device->new_compute_pipeline_state(ps_desc));
				}

				//Lum Pass.
				{
					luset(m_lum_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
						DescriptorSetLayoutBinding(DescriptorType::srv, 0, 1, ShaderVisibility::all),
						DescriptorSetLayoutBinding(DescriptorType::uav, 1, 1, ShaderVisibility::all)
						})));
					luset(m_lum_pass_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ m_lum_pass_dlayout },
						ShaderInputLayoutFlag::deny_vertex_shader_access |
						ShaderInputLayoutFlag::deny_domain_shader_access |
						ShaderInputLayoutFlag::deny_geometry_shader_access |
						ShaderInputLayoutFlag::deny_hull_shader_access |
						ShaderInputLayoutFlag::deny_pixel_shader_access)));

					lulet(psf, open_file("LumCS.cso", FileOpenFlag::read, FileCreationMode::open_existing));
					auto file_size = psf->get_size();
					auto cs_blob = Blob((usize)file_size);
					luexp(psf->read(cs_blob.data(), cs_blob.size()));
					psf = nullptr;
					ComputePipelineStateDesc ps_desc;
					ps_desc.cs = cs_blob.cspan();
					ps_desc.shader_input_layout = m_lum_pass_slayout;
					luset(m_lum_pass_pso, device->new_compute_pipeline_state(ps_desc));
				}

				//Tone Mapping Pass.
				{
					luset(m_tone_mapping_pass_dlayout, device->new_descriptor_set_layout(DescriptorSetLayoutDesc({
						DescriptorSetLayoutBinding(DescriptorType::cbv, 0, 1, ShaderVisibility::all),
						DescriptorSetLayoutBinding(DescriptorType::srv, 1, 1, ShaderVisibility::all),
						DescriptorSetLayoutBinding(DescriptorType::srv, 2, 1, ShaderVisibility::all),
						DescriptorSetLayoutBinding(DescriptorType::uav, 3, 1, ShaderVisibility::all)
						})));
					luset(m_tone_mapping_pass_slayout, device->new_shader_input_layout(ShaderInputLayoutDesc({ m_tone_mapping_pass_dlayout },
						ShaderInputLayoutFlag::deny_vertex_shader_access |
						ShaderInputLayoutFlag::deny_domain_shader_access |
						ShaderInputLayoutFlag::deny_geometry_shader_access |
						ShaderInputLayoutFlag::deny_hull_shader_access |
						ShaderInputLayoutFlag::deny_pixel_shader_access)));

					lulet(psf, open_file("ToneMappingCS.cso", FileOpenFlag::read, FileCreationMode::open_existing));
					auto file_size = psf->get_size();
					auto cs_blob = Blob((usize)file_size);
					luexp(psf->read(cs_blob.data(), cs_blob.size()));
					psf = nullptr;
					ComputePipelineStateDesc ps_desc;
					ps_desc.cs = cs_blob.cspan();
					ps_desc.shader_input_layout = m_tone_mapping_pass_slayout;
					luset(m_tone_mapping_pass_pso, device->new_compute_pipeline_state(ps_desc));
				}
		}
		lucatchret;
		return ok;
	}
	static Ref<IAssetEditor> new_scene_editor(object_t userdata, Asset::asset_t editing_asset)
	{
		auto edit = new_object<SceneEditor>();
		edit->m_type = ObjRef(userdata);
		edit->m_scene = editing_asset;
		lupanic_if_failed(edit->init());
		return edit;
	}

	RV register_scene_editor()
	{
		lutry
		{
			register_boxed_type<SceneEditorUserData>();
			register_boxed_type<SceneEditor>();
			impl_interface_for_type<SceneEditor, IAssetEditor>();

			AssetEditorDesc desc;
			desc.new_editor = new_scene_editor;
			desc.on_draw_tile = nullptr;
			auto userdata = new_object<SceneEditorUserData>();
			luexp(userdata->init());
			desc.userdata = userdata;
			g_env->register_asset_editor_type(get_scene_asset_type(), desc);
		}
		lucatchret;
		return ok;
	}


	struct SceneCreator : public IAssetEditor
	{
		lustruct("SceneCreator", "{B91FE406-7281-43F5-9688-2C6CFF17BED2}");
		luiimpl();

		Path m_create_dir;
		String m_asset_name;
		bool m_open;

		SceneCreator() :
			m_open(true)
		{
			m_asset_name = String();
		}

		virtual void on_render() override;
		virtual bool closed() override
		{
			return !m_open;
		}
	};

	void SceneCreator::on_render()
	{
		char title[32];
		sprintf_s(title, "Create Scene###%d", (u32)(usize)this);

		ImGui::Begin(title, &m_open, ImGuiWindowFlags_NoCollapse);

		ImGui::InputText("Scene Asset Name", m_asset_name);
		if (!m_asset_name.empty())
		{
			ImGui::Text("The Scene will be created as: %s%s", m_create_dir.encode().c_str(), m_asset_name.c_str());
			if (ImGui::Button("Create"))
			{
				lutry
				{
					Path asset_path = m_create_dir;
					asset_path.push_back(m_asset_name);
					lulet(asset, Asset::new_asset(asset_path, get_scene_asset_type()));
					Ref<Scene> s = new_object<Scene>();
					luexp(Asset::set_asset_data(asset, s.object()));
					luexp(Asset::save_asset(asset));
				}
				lucatch
				{
					auto _ = Window::message_box(explain(lures), "Failed to create scene asset",
									Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
				}
			}
		}

		ImGui::End();
	}

	static Ref<IAssetEditor> new_scene_importer(const Path& create_dir)
	{
		auto dialog = new_object<SceneCreator>();
		dialog->m_create_dir = create_dir;
		return dialog;
	}

	void register_scene_importer()
	{
		register_boxed_type<SceneCreator>();
		impl_interface_for_type<SceneCreator, IAssetEditor>();
		AssetImporterDesc desc;
		desc.new_importer = new_scene_importer;
		g_env->register_asset_importer_type(get_scene_asset_type(), desc);
	}
}