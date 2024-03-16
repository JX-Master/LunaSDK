/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SceneEditor.cpp
* @author JXMaster
* @date 2020/5/15
*/
#include "Scene.hpp"
#include "../SceneRenderer.hpp"
#include "../MainEditor.hpp"
#include "../Scene.hpp"
#include <Luna/ShaderCompiler/ShaderCompiler.hpp>
#include <Luna/RHI/ShaderCompileHelper.hpp>
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/Window/MessageBox.hpp>
#include <Luna/HID/Mouse.hpp>
#include "../EditObject.hpp"
#include "../SceneSettings.hpp"
#include "../Camera.hpp"
#include <Luna/Window/FileDialog.hpp>
#include <Luna/Window/MessageBox.hpp>
#include <Luna/RHI/Utility.hpp>
#include <Luna/Image/RHIHelper.hpp>
namespace Luna
{
    struct SceneEditorUserData
    {
        lustruct("SceneEditorUserData", "{5b4aea33-e61a-4042-ba91-1f4ec84f8194}");

        // Resources for rendering grids.
        Ref<RHI::IBuffer> m_grid_vb;
        Ref<RHI::IDescriptorSetLayout> m_grid_dlayout;
        Ref<RHI::IPipelineLayout> m_grid_playout;
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

        SceneRenderer m_renderer;

        // States for entity list.
        i32 m_new_entity_current_item = 0;
        u32 m_current_select_entity = 0;
        bool m_name_editing = false;
        String m_name_editing_buf;

        // States for component grid.
        WeakRef<Entity> m_current_entity;

        // States for scene viewport.

        ImGui::GizmoMode m_gizmo_mode = ImGui::GizmoMode::local;
        ImGui::GizmoOperation m_gizmo_op = ImGui::GizmoOperation::translate;

        f32 m_camera_speed = 1.0f;

        bool m_navigating = false;
        Int2U m_scene_click_pos;    // Stores the click mouse position in screen space.

        bool m_open = true;

        SceneEditor() :
            m_renderer(RHI::get_main_device()) {}

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

        void capture_scene_to_file(const Path& path);
    };

    RV SceneEditor::init()
    {
        lutry
        {
            using namespace RHI;
            auto device = get_main_device();
            auto cb_align = device->check_feature(DeviceFeature::uniform_buffer_data_alignment).uniform_buffer_data_alignment;
            luset(m_renderer.command_buffer, g_env->device->new_command_buffer(g_env->graphics_queue));
            SceneRendererSettings settings;
            settings.frame_profiling = true;
            settings.mode = SceneRendererMode::lit;
            settings.screen_size = UInt2U(1024, 768);
            luexp(m_renderer.reset(settings));
        }
        lucatchret;
        return ok;
    }

    void SceneEditor::draw_entity_list()
    {
        auto s = get_asset_or_async_load_if_not_ready<Scene>(m_scene);

        // Draw entity list.
        ImGui::Text("Entity List");

        ImGui::SameLine();

        if (ImGui::Button("New Entity"))
        {
            char name[64];
            strncpy(name, "New_Entity", 64);
            auto entity = s->add_entity(Name(name));
            if (entity.errcode() == BasicError::already_exists())
            {
                u32 index = 0;
                // Append index.
                while (failed(entity))
                {
                    snprintf(name, 64, "New_Entity_%u", index);
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
                            Color::to_rgba8(Float4(ImGui::GetStyle().Colors[(u32)ImGuiCol_Button])));
                    }
                    ImGui::Text("%s", entities[i]->name.c_str());
                }

                if (ImGui::IsWindowFocused() && in_bounds(ImGui::GetIO().MousePos, sel_pos, sel_pos + sel_size) && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                {
                    ImGui::OpenPopup(entity_popup_id);
                }

            }

            if (ImGui::BeginPopup(entity_popup_id))
            {
                if (ImGui::Selectable("Rename"))
                {
                    m_name_editing = true;
                    m_name_editing_buf = s->root_entities[m_current_select_entity]->name.c_str();
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

        auto s = get_asset_or_async_load_if_not_ready<Scene>(m_scene);
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
                else
                {
                    ++iter;
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
            // Collect last frame profiling data.
            if(m_renderer.get_settings().frame_profiling)
            {
                m_renderer.collect_frame_profiling_data();
            }

            ImGui::Text("Scene");

            Scene* s = get_asset_or_async_load_if_not_ready<Scene>(m_scene);

            m_renderer.scene = s;

            auto r = m_renderer.render();
            if(failed(r))
            {
                ImGui::Text("%s", explain(r.errcode()));
                return ok;
            }

            ImGui::BeginChild("Scene Viewport", Float2(0.0f, 0.0f), false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

            ImGui::SetNextItemWidth(100.0f);
            ImGui::SliderFloat("Camera Speed", &m_camera_speed, 0.1f, 10.0f, "%.3f");
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

            auto settings = m_renderer.get_settings();

            ImGui::SameLine();
            ImGui::SetNextItemWidth(150.0f);
            {
                auto render_mode_type = typeof<SceneRendererMode>();
                auto options = get_enum_options(render_mode_type);
                Name current_name;
                for(auto& option : options)
                {
                    if(option.value == (i64)settings.mode)
                    {
                        current_name = option.name;
                        break;
                    }
                }
                if(ImGui::BeginCombo("Render Mode", current_name.c_str()))
                {
                    for(auto& option : options)
                    {
                        bool selected = (option.value == (i64)settings.mode);
                        if(ImGui::Selectable(option.name.c_str(), selected))
                        {
                            settings.mode = (SceneRendererMode)option.value;
                        }
                    }
                    ImGui::EndCombo();
                }
            }
            ImGui::SameLine();
            ImGui::Checkbox("Time Profiling", &settings.frame_profiling);

            Float2 scene_sz = ImGui::GetContentRegionAvail();
            Float2 scene_pos = ImGui::GetCursorScreenPos();
            scene_sz.x -= 1.0f;
            scene_sz.y -= 5.0f;

            auto& io = ImGui::GetIO();

            settings.screen_size = UInt2U((u32)(scene_sz.x * io.DisplayFramebufferScale.x), (u32)(scene_sz.y * io.DisplayFramebufferScale.y));

            // Draw Overlays.
            luexp(m_renderer.command_buffer->submit({}, {}, true));

            ImGui::Image(m_renderer.render_texture, scene_sz);

            auto scene_renderer = s->get_scene_component<SceneSettings>();
            auto camera_entity = s->find_entity(scene_renderer->camera_entity);
            auto camera_component = camera_entity->get_component<Camera>();

            // Draw GUI Overlays.
            {
                // Draw gizmo.
                auto e = m_current_entity.pin();
                if (e && e != camera_entity)
                {
                    Float4x4 world_mat = e->local_to_world_matrix();
                    bool edited = false;
                    ImGui::Gizmo(world_mat, camera_entity->world_to_local_matrix(), camera_component->get_projection_matrix(),
                        RectF(scene_pos.x, scene_pos.y, scene_sz.x, scene_sz.y), m_gizmo_op, m_gizmo_mode, 0.0f, true, false, nullptr, nullptr, &edited);
                    if (edited)
                    {
                        e->set_local_to_world_matrix(world_mat);
                    }
                }

                // Draw scene debug info.
                auto backup_pos = ImGui::GetCursorPos();
                ImGui::SetCursorScreenPos(scene_pos);

                if (m_renderer.get_settings().frame_profiling)
                {
                    ImGui::Text("Frame Size: %ux%u", (u32)(scene_sz.x * io.DisplayFramebufferScale.x), (u32)(scene_sz.y * io.DisplayFramebufferScale.y));
                    ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
                    for (usize i = 0; i < m_renderer.pass_time_intervals.size(); ++i)
                    {
                        f64 interval = m_renderer.pass_time_intervals[i];
                        if(interval < 0.001)
                        {
                            ImGui::Text("%s: %fus", m_renderer.enabled_passes[i].c_str(), m_renderer.pass_time_intervals[i] * 1000000.0);
                        }
                        else
                        {
                            ImGui::Text("%s: %fms", m_renderer.enabled_passes[i].c_str(), m_renderer.pass_time_intervals[i] * 1000.0);
                        }
                    }
                }
                ImGui::SetCursorPos(backup_pos);

                // Draw scene gizmos.
                if(camera_entity)
                {
                    f32 gizmo_size = 50.0f;
                    f32 gizmo_len = gizmo_size;

                    Float4x4 view_mat = inverse(AffineMatrix::make(camera_entity->position, camera_entity->rotation, Float3(1.0f)));
                    Float4 x_gizmo = mul(Float4(1.0f, 0.0f, 0.0f, 0.0f) * gizmo_len, view_mat);
                    Float4 y_gizmo = mul(Float4(0.0f, 1.0f, 0.0f, 0.0f) * gizmo_len, view_mat);
                    Float4 z_gizmo = mul(Float4(0.0f, 0.0f, 1.0f, 0.0f) * gizmo_len, view_mat);

                    ImDrawList* dw = ImGui::GetWindowDrawList();
                    Float2 origin_point = { scene_pos.x + gizmo_size, scene_pos.y + scene_sz.y - gizmo_size };

                    struct GizmoLine
                    {
                        Float3U line;
                        u32 color;

                        bool operator<(const GizmoLine& rhs) const
                        {
                            // Higher depth value gets drawn first (appear first in the draw list).
                            return line.z > rhs.line.z;
                        }
                    };

                    Vector<GizmoLine> lines;
                    lines.push_back({ x_gizmo.xyz(), Color::to_rgba8(Color::red()) });
                    lines.push_back({ y_gizmo.xyz(), Color::to_rgba8(Color::green()) });
                    lines.push_back({ z_gizmo.xyz(), Color::to_rgba8(Color::blue()) });

                    // Sort by depth to ensure correct drawing order.
                    sort(lines.begin(), lines.end());
                    for (auto& line : lines)
                    {
                        // Revert y axis, since y axis of ImGui points to down, not up.
                        dw->AddLine(origin_point, origin_point + Float2(line.line.x, -line.line.y), line.color, 5.0f);
                    }
                }
            }

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && in_bounds(ImGui::GetIO().MousePos, scene_pos, scene_pos + scene_sz))
            {
                m_navigating = true;
                m_scene_click_pos = HID::get_mouse_pos();
            }

            if (m_navigating && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
            {
                m_navigating = false;
            }

            if (m_navigating)
            {
                auto mouse_pos = HID::get_mouse_pos();
                auto mouse_delta = mouse_pos - m_scene_click_pos;
                m_scene_click_pos = mouse_pos;
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
                auto eular = AffineMatrix::euler_angles(rot_mat);
                eular += {deg_to_rad((f32)mouse_delta.y / 10.0f), deg_to_rad((f32)mouse_delta.x / 10.0f), 0.0f};
                eular.x = clamp(eular.x, deg_to_rad(-85.0f), deg_to_rad(85.0f));
                camera_entity->rotation = Quaternion::from_euler_angles(eular);
            }
            m_renderer.command_buffer->wait();
            luassert_always(succeeded(m_renderer.command_buffer->reset()));
            if(settings != m_renderer.get_settings())
            {
                luexp(m_renderer.reset(settings));
            }
            ImGui::EndChild();
        }
        lucatchret;
        return ok;

    }

    static void draw_transform(Entity* t)
    {
        ImGui::DragFloat3("Position", t->position.m, 0.01f);

        auto euler = AffineMatrix::euler_angles(AffineMatrix::make_rotation(t->rotation));
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
                    else
                    {
                        ++iter;
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
        snprintf(title, 32, "Scene Editor###%d", (u32)(usize)this);
        ImGui::SetNextWindowSize(Float2(1000, 500), ImGuiCond_FirstUseEver);
        ImGui::Begin(title, &m_open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);
        auto s = get_asset_or_async_load_if_not_ready<Scene>(m_scene);
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

        bool capture_scene = false;
        Path capture_save_path;

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
                        auto _ = Window::message_box(explain(luerr), "Failed to save scene", Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
                    }
                }
                ImGui::EndMenu();
            }
            if(ImGui::BeginMenu("Tools"))
            {
                if(ImGui::MenuItem("Capture scene"))
                {
                    Window::FileDialogFilter filter;
                    filter.name = "BMP File";
                    const c8* ext = "bmp";
                    filter.extensions = {&ext, 1};
                    auto r = Window::save_file_dialog("Save Capture File", {&filter, 1});
                    if(succeeded(r))
                    {
                        capture_scene = true;
                        capture_save_path = r.get();
                        capture_save_path.replace_extension("bmp");
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

        RV r = draw_scene();
        if(failed(r))
        {
            auto _ = Window::message_box(explain(r.errcode()), "Scene Editor Error", Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
        }

        ImGui::NextColumn();

        draw_components_grid();

        ImGui::NextColumn();

        ImGui::End();

        if(capture_scene)
        {
            capture_scene_to_file(capture_save_path);
        }
    }

    void SceneEditor::capture_scene_to_file(const Path& path)
    {
        using namespace RHI;
        lutry
        {
            auto device = g_env->device;
            auto desc = m_renderer.render_texture->get_desc();
            usize row_pitch = bits_per_pixel(desc.format) * (usize)desc.width / 8;
            usize slice_pitch = row_pitch * desc.height;
            Blob img_data(slice_pitch);
            lulet(readback_cmdbuf, device->new_command_buffer(g_env->async_copy_queue));
            luexp(copy_resource_data(readback_cmdbuf, {CopyResourceData::read_texture(img_data.data(), (u32)row_pitch, (u32)slice_pitch, m_renderer.render_texture, SubresourceIndex(0, 0), 0, 0, 0,
                desc.width, desc.height, 1)}));
            Image::ImageDesc img_desc;
            img_desc.width = (u32)desc.width;
            img_desc.height = desc.height;
            img_desc.format = Image::rhi_to_image_format(desc.format);
            luassert(img_desc.format != Image::ImageFormat::unkonwn);
            lulet(f, open_file(path.encode().c_str(), FileOpenFlag::write | FileOpenFlag::user_buffering, FileCreationMode::create_always));
            luexp(Image::write_bmp_file(f, img_desc, img_data));
        }
        lucatch
        {
            auto _ = Window::message_box(explain(luerr), "Failed to capture image", Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
        }
    }

    RV SceneEditorUserData::init()
    {
        //! Initialize Grid data.
        Float4U grids[44];
        for (i32 i = -5; i <= 5; ++i) // 0 - 21
        {
            grids[(i + 5) * 2] = Float4U((f32)i, 0.0f, 5.0f, 1.0f);
            grids[(i + 5) * 2 + 1] = Float4U((f32)i, 0.0f, -5.0f, 1.0f);
        }
        for (i32 i = -5; i <= 5; ++i) // 22 - 43
        {
            grids[(i + 5) * 2 + 22] = Float4U(-5.0f, 0.0f, (f32)i, 1.0f);
            grids[(i + 5) * 2 + 23] = Float4U(5.0f, 0.0f, (f32)i, 1.0f);
        }

        lutry
        {
            using namespace RHI;
            auto device = get_main_device();
            {
                luset(m_grid_vb, device->new_buffer(MemoryType::local, BufferDesc(BufferUsageFlag::copy_dest | BufferUsageFlag::vertex_buffer, sizeof(grids))));

                DescriptorSetLayoutBinding dlayout_binding = DescriptorSetLayoutBinding::uniform_buffer_view(0, 1, ShaderVisibilityFlag::vertex);
                DescriptorSetLayoutDesc dlayout({&dlayout_binding, 1});
                luset(m_grid_dlayout, device->new_descriptor_set_layout(dlayout));
                auto dl = m_grid_dlayout.get();
                luset(m_grid_playout, device->new_pipeline_layout(PipelineLayoutDesc({ &dl, 1 },
                    PipelineLayoutFlag::allow_input_assembler_input_layout)));
                static const char* vertexShader =
                    R"(cbuffer vertexBuffer : register(b0)
                        {
                            float4x4 world_to_view;
                            float4x4 view_to_proj;
                            float4x4 world_to_proj;
                            float4x4 view_to_world;
                        };
                        struct VS_INPUT
                        {
                          [[vk::location(0)]]
                          float4 pos : POSITION;
                        };
                        
                        struct PS_INPUT
                        {
                          [[vk::location(0)]]
                          float4 pos : SV_POSITION;
                        };
                        
                        PS_INPUT main(VS_INPUT input)
                        {
                          PS_INPUT output;
                          output.pos = mul(world_to_proj, input.pos);
                          return output;
                        })";
                auto compiler = ShaderCompiler::new_compiler();
                ShaderCompiler::ShaderCompileParameters params;
                params.source = { vertexShader, strlen(vertexShader) };
                params.source_name = "GridVS";
                params.entry_point = "main";
                params.target_format = get_current_platform_shader_target_format();
                params.shader_type = ShaderCompiler::ShaderType::vertex;
                params.shader_model = {6, 0};
                params.optimization_level = ShaderCompiler::OptimizationLevel::full;
                lulet(vs_blob, compiler->compile(params));
                static const char* pixelShader =
                    R"(struct PS_INPUT
                    {
                        [[vk::location(0)]]
                        float4 pos : SV_POSITION;
                    };
                    [[vk::location(0)]]
                    float4 main(PS_INPUT input) : SV_Target
                    {
                        return float4(1.0f, 1.0f, 1.0f, 1.0f);
                    })";

                params.source = { pixelShader, strlen(pixelShader) };
                params.source_name = "GridPS";
                params.entry_point = "main";
                params.target_format = get_current_platform_shader_target_format();
                params.shader_type = ShaderCompiler::ShaderType::pixel;
                params.shader_model = {6, 0};
                params.optimization_level = ShaderCompiler::OptimizationLevel::full;
                lulet(ps_blob, compiler->compile(params));

                GraphicsPipelineStateDesc ps_desc;
                ps_desc.primitive_topology = PrimitiveTopology::line_list;
                ps_desc.blend_state = BlendDesc({ AttachmentBlendDesc(true, BlendFactor::src_alpha,
                    BlendFactor::one_minus_src_alpha, BlendOp::add, BlendFactor::one_minus_src_alpha, BlendFactor::zero, BlendOp::add, ColorWriteMask::all) });
                ps_desc.rasterizer_state = RasterizerDesc(FillMode::wireframe, CullMode::none, 0.0f, 0.0f, 0.0f, false, true);
                ps_desc.depth_stencil_state = DepthStencilDesc(false, false, CompareFunction::always, false, 0x00, 0x00, DepthStencilOpDesc(), DepthStencilOpDesc());
                ps_desc.ib_strip_cut_value = IndexBufferStripCutValue::disabled;
                auto attribute = InputAttributeDesc("POSITION", 0, 0, 0, 0, Format::rgba32_float);
                auto binding = InputBindingDesc(0, sizeof(Float4U), InputRate::per_vertex);
                ps_desc.input_layout.attributes = { &attribute, 1 };
                ps_desc.input_layout.bindings = { &binding, 1 };
                ps_desc.pipeline_layout = m_grid_playout;
                ps_desc.vs = get_shader_data_from_compile_result(vs_blob);
                ps_desc.ps = get_shader_data_from_compile_result(ps_blob);
                ps_desc.num_color_attachments = 1;
                ps_desc.color_formats[0] = Format::rgba8_unorm;

                luset(m_grid_pso, device->new_graphics_pipeline_state(ps_desc));
            }

            // Upload grid vertex data.
            lulet(upload_cmdbuf, device->new_command_buffer(g_env->async_copy_queue));
            luexp(copy_resource_data(upload_cmdbuf, {CopyResourceData::write_buffer(m_grid_vb, 0, grids, sizeof(grids))}));
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
}
