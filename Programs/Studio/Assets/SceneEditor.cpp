/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SceneEditor.cpp
* @author JXMaster
* @date 2020/5/15
*/
#include "SceneEditor.hpp"
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
#include "../RenderPasses/LightingPass.hpp"
#include "../RenderPasses/SkyBoxPass.hpp"
#include "../RenderPasses/ToneMappingPass.hpp"
#include "../RenderPasses/WireframePass.hpp"
namespace Luna
{
	struct SceneEditorUserData
	{
		lustruct("SceneEditorUserData", "{5b4aea33-e61a-4042-ba91-1f4ec84f8194}");

		// Resources for rendering grids.
		Ref<RHI::IResource> m_grid_vb;
		Ref<RHI::IDescriptorSetLayout> m_grid_dlayout;
		Ref<RHI::IShaderInputLayout> m_grid_slayout;
		Ref<RHI::IPipelineState> m_grid_pso;

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

		Ref<RG::IRenderGraph> m_render_graph;

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

		Ref<RHI::IResource> m_lighting_params;
		usize m_num_lights = 0;

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
			luset(m_scene_cmdbuf, g_env->graphics_queue->new_command_buffer());

			luset(m_grid_desc_set, device->new_descriptor_set(DescriptorSetDesc(m_type->m_grid_dlayout)));
			m_grid_desc_set->set_cbv(0, m_camera_cb, ConstantBufferViewDesc(0, (u32)align_upper(sizeof(CameraCB), cb_align)));
			m_render_graph = RG::new_render_graph(device);
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
				if (in_bounds(ImGui::GetIO().MousePos, sel_pos, sel_pos + sel_size) && !ImGui::IsPopupOpen(entity_popup_id) &&
					(ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
				{
					m_current_select_entity = i;
					m_current_entity = entities[i];
				}
				if (i == m_current_select_entity && m_name_editing)
				{
					ImGui::InputText("###NameEdit", m_name_editing_buf);
					if (!in_bounds(ImGui::GetIO().MousePos, sel_pos, sel_pos + sel_size) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
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

				if (in_bounds(ImGui::GetIO().MousePos, sel_pos, sel_pos + sel_size) && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
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

			camera_component->aspect_ratio = scene_sz.x / scene_sz.y;

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

			RHI::IResource* render_tex = nullptr;
			RHI::IRenderTargetView* render_rtv = nullptr;

			// Draw Scene.
			{
				using namespace RHI;

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
							p.direction = AffineMatrix::forward(AffineMatrix::make_rotation(light_ts[i]->world_rotation()));
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
									p.direction = AffineMatrix::forward(AffineMatrix::make_rotation(light_ts[i]->world_rotation()));
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

				// Resources.
				constexpr usize LIGHTING_BUFFER = 0;
				constexpr usize DEPTH_BUFFER = 1;
				constexpr usize BACK_BUFFER = 2;
				constexpr usize WIREFRAME_BACK_BUFFER = 3;

				// Passes.
				constexpr usize WIREFRAME_PASS = 0;
				constexpr usize SKYBOX_PASS = 1;
				constexpr usize LIGHTING_PASS = 2;
				constexpr usize TONE_MAPPING_PASS = 3;
				
				// Build render graph.
				{
					using namespace RG;
					
					
					RenderGraphDesc desc;
					desc.passes.resize(4);
					desc.passes[WIREFRAME_PASS] = {"WireframePass", "Wireframe"};
					desc.passes[SKYBOX_PASS] = {"SkyBoxPass", "SkyBox"};
					desc.passes[LIGHTING_PASS] = {"LightingPass", "Lighting"};
					desc.passes[TONE_MAPPING_PASS] = {"ToneMappingPass", "ToneMapping"};
					desc.resources.resize(4);
					desc.resources[LIGHTING_BUFFER] = {RenderGraphResourceType::internal, "LightingBuffer"};
					desc.resources[DEPTH_BUFFER] = {RenderGraphResourceType::internal, "DepthBuffer"};
					desc.resources[BACK_BUFFER] = {RenderGraphResourceType::internal, "BackBuffer"};
					desc.resources[WIREFRAME_BACK_BUFFER] = {RenderGraphResourceType::internal, "WireframeBackBuffer"};
					if(m_wireframe)
					{
						desc.resources[WIREFRAME_BACK_BUFFER].type = RenderGraphResourceType::output;
					}
					else
					{
						desc.resources[BACK_BUFFER].type = RenderGraphResourceType::output;
					}
					desc.output_connections.push_back({WIREFRAME_PASS, "scene_texture", WIREFRAME_BACK_BUFFER});
					desc.output_connections.push_back({SKYBOX_PASS, "texture", LIGHTING_BUFFER});
					desc.output_connections.push_back({LIGHTING_PASS, "scene_texture", LIGHTING_BUFFER});
					desc.output_connections.push_back({LIGHTING_PASS, "scene_depth_texture", DEPTH_BUFFER});
					desc.input_connections.push_back({TONE_MAPPING_PASS, "hdr_texture", LIGHTING_BUFFER});
					desc.output_connections.push_back({TONE_MAPPING_PASS, "ldr_texture", BACK_BUFFER});
					desc.resource_descs.push_back({LIGHTING_BUFFER, ResourceDesc::tex2d(ResourceHeapType::local, Format::rgba16_float, 
						ResourceUsageFlag::shader_resource | ResourceUsageFlag::render_target, (u32)scene_sz.x, (u32)scene_sz.y)});
					desc.resource_descs.push_back({DEPTH_BUFFER, ResourceDesc::tex2d(ResourceHeapType::local, Format::d32_float, 
						ResourceUsageFlag::shader_resource | ResourceUsageFlag::render_target, (u32)scene_sz.x, (u32)scene_sz.y)});
					desc.resource_descs.push_back({WIREFRAME_BACK_BUFFER, ResourceDesc::tex2d(ResourceHeapType::local, Format::rgba8_unorm, 
						ResourceUsageFlag::shader_resource | ResourceUsageFlag::render_target, (u32)scene_sz.x, (u32)scene_sz.y)});
					m_render_graph->set_desc(desc);
					luexp(m_render_graph->compile());
				}

				{
					// Set parameters.
					if(m_wireframe)
					{
						WireframePass* wireframe = query_interface<WireframePass>(m_render_graph->get_render_pass(WIREFRAME_PASS)->get_object());
						wireframe->model_matrices = m_model_matrices;
						wireframe->camera_cb = m_camera_cb;
						wireframe->ts = {ts.data(), ts.size()};
						wireframe->rs = {rs.data(), rs.size()};
					}
					else
					{
						SkyBoxPass* skybox = query_interface<SkyBoxPass>(m_render_graph->get_render_pass(SKYBOX_PASS)->get_object());
						LightingPass* lighting = query_interface<LightingPass>(m_render_graph->get_render_pass(LIGHTING_PASS)->get_object());
						ToneMappingPass* tone_mapping = query_interface<ToneMappingPass>(m_render_graph->get_render_pass(TONE_MAPPING_PASS)->get_object());
						skybox->camera_fov = camera_component->fov;
						skybox->camera_type = camera_component->type;
						skybox->view_to_world = camera_entity->local_to_world_matrix();
						auto skybox_tex = Asset::get_asset_data<RHI::IResource>(scene_renderer->skybox);
						skybox->skybox = skybox_tex;
						lighting->skybox = skybox_tex;
						lighting->model_matrices = m_model_matrices;
						lighting->camera_cb = m_camera_cb;
						lighting->ts = {ts.data(), ts.size()};
						lighting->rs = {rs.data(), rs.size()};
						lighting->light_params = m_lighting_params;
						lighting->light_ts = {light_ts.data(), light_ts.size()};
						tone_mapping->exposure = scene_renderer->exposure;
					}
				}

				luexp(m_render_graph->execute(m_scene_cmdbuf));

				// Set render pass parameters.
				{
					if(m_wireframe)
					{
						render_tex = m_render_graph->get_output_resource(WIREFRAME_BACK_BUFFER);
					}
					else
					{
						render_tex = m_render_graph->get_output_resource(BACK_BUFFER);
					}
					luset(render_rtv, m_render_graph->get_device()->new_render_target_view(render_tex));
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
				m_scene_cmdbuf->set_viewport(Viewport(0.0f, 0.0f, scene_sz.x, scene_sz.y, 0.0f, 1.0f));
				m_scene_cmdbuf->set_scissor_rect(RectI(0, 0, (i32)scene_sz.x, (i32)scene_sz.y));
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

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && in_bounds(ImGui::GetIO().MousePos, scene_pos, scene_pos + scene_sz))
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
				auto rot_mat = AffineMatrix::make_rotation(rot);

				// Key control.
				auto left = AffineMatrix::left(rot_mat);
				auto forward = AffineMatrix::forward(rot_mat);
				auto up = AffineMatrix::up(rot_mat);

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

		auto euler = AffineMatrix::make_rotation(t->rotation).euler_angles();
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

			luexp(register_sky_box_pass());
			luexp(register_wireframe_pass());
			luexp(register_lighting_pass());
			luexp(register_tone_mapping_pass());
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