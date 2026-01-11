/*!
* This file is a portion of LunaSDK.
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
#include <Luna/RHIUtility/ResourceReadContext.hpp>
#include <Luna/RHIUtility/ResourceWriteContext.hpp>
#include <Luna/Image/RHIHelper.hpp>
#include <Luna/Runtime/Random.hpp>
#include "../World.hpp"

#include <GridVS.hpp>
#include <GridPS.hpp>

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
        World m_world;
        bool m_world_initialized = false;

        SceneRenderer m_renderer;

        // States for actor list.
        Guid m_editing_actor_guid = Guid(0, 0);

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

        void on_add_actor(usize actor_index);
        void on_remove_actor(const Guid& guid);
        void on_edit_actor_info(SceneActor& scene_actor);
        void on_edit_actor_transform(SceneActor& scene_actor);
        void on_actor_add_component(SceneActor& scene_actor, typeinfo_t component);
        void on_actor_remove_component(SceneActor& scene_actor, typeinfo_t component);
        void on_actor_edit_component(SceneActor& scene_actor, typeinfo_t component);

        void edit_scene();
        void draw_actor_list();
        void draw_actor_tree_node(Actor* actor, bool& open_actor_list_popup);
        void draw_scene_settings();
        void draw_scene();

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

    static void write_scene_actor(SceneActor& dst, const Actor* src)
    {
        const ActorInfo* info = src->get_component<ActorInfo>();
        dst.guid = info->get_guid();
        dst.name = info->name;
        const Transform* transform = src->get_transform();
        dst.transform = *transform;
        dst.children.clear();
        Vector<Actor*> children;
        info->get_children(children);
        for(auto& child : children)
        {
            dst.children.push_back(child->get_actor_info()->get_guid());
        }
        dst.components.clear();
        auto addr = src->get_world()->get_entity_address(src->get_entity()).get();
        auto components = ECS::get_cluster_components(addr.cluster);
        for(typeinfo_t type : components)
        {
            if(type == typeof<ActorInfo>() || type == typeof<Transform>()) continue;
            object_t obj = object_alloc(type);
            const void* src = (const void*)ECS::get_cluster_components_data(addr.cluster, addr.index / ECS::CLUSTER_CHUNK_CAPACITY, type);
            src = (const void*)((usize)src + get_type_size(type) * (addr.index % ECS::CLUSTER_CHUNK_CAPACITY));
            copy_construct_type(type, obj, src);
            ObjRef ref;
            ref.attach(obj);
            dst.components.push_back(move(ref));
        }
    }

    void SceneEditor::on_add_actor(usize actor_index)
    {
        auto s = get_asset_or_async_load_if_not_ready<Scene>(m_scene);
        auto& scene_actor = s->actors[actor_index];
        Vector<typeinfo_t> components;
        components.reserve(scene_actor.components.size());
        for(auto& obj : scene_actor.components)
        {
            components.push_back(obj.type());
        }
        Actor* actor = m_world.add_actor(scene_actor.guid, components.cspan(), {s.get()});
        auto info = actor->get_actor_info();
        auto transform = actor->get_transform();
        info->name = scene_actor.name;
        *transform = scene_actor.transform;
    }
    void SceneEditor::on_remove_actor(const Guid& guid)
    {
        m_world.remove_actor(guid);
    }
    void SceneEditor::on_edit_actor_info(SceneActor& scene_actor)
    {
        Actor* actor = m_world.get_actor(scene_actor.guid);
        auto info = actor->get_actor_info();
        info->name = scene_actor.name;
    }
    void SceneEditor::on_edit_actor_transform(SceneActor& scene_actor)
    {
        Actor* actor = m_world.get_actor(scene_actor.guid);
        auto transform = actor->get_transform();
        *transform = scene_actor.transform;
    }
    void SceneEditor::on_actor_add_component(SceneActor& scene_actor, typeinfo_t component)
    {
        Actor* actor = m_world.get_actor(scene_actor.guid);
        void* data = actor->add_component(component);
        void* src = nullptr;
        for(auto& obj : scene_actor.components)
        {
            if(obj.type() == component)
            {
                src = obj.get();
                break;
            }
        }
        luassert(src);
        copy_assign_type(component, data, src);
    }
    void SceneEditor::on_actor_remove_component(SceneActor& scene_actor, typeinfo_t component)
    {
        Actor* actor = m_world.get_actor(scene_actor.guid);
        actor->remove_component(component);
    }
    void SceneEditor::on_actor_edit_component(SceneActor& scene_actor, typeinfo_t component)
    {
        Actor* actor = m_world.get_actor(scene_actor.guid);
        void* data = actor->get_component(component);
        void* src = nullptr;
        for(auto& obj : scene_actor.components)
        {
            if(obj.type() == component)
            {
                src = obj.get();
                break;
            }
        }
        luassert(src);
        copy_assign_type(component, data, src);
    }

    void SceneEditor::draw_actor_list()
    {
        auto s = get_asset_or_async_load_if_not_ready<Scene>(m_scene);

        // Draw  list.
        ImGui::Text("Actor List");

        ImGui::SameLine();

        if (ImGui::Button("New Actor"))
        {
            auto iter = s->actors.emplace_back();
            iter->guid = random_guid();
            iter->name = "New Actor";
            on_add_actor(s->actors.size() - 1);
        }

        auto avail = ImGui::GetContentRegionAvail();

        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
        ImGui::BeginChild("Actor List", Float2(avail.x, avail.y / 2.0f), true);

        if (s->actors.empty())
        {
            ImGui::Text("No actor in the scene.");
        }
        else
        {
            const char* actor_popup_id = "Entity Popup";

            bool open_actor_list_popup = false;

            for(auto& actor : s->actors)
            {
                Actor* a = m_world.get_actor(actor.guid);
                if(a->get_actor_info()->get_parent() == nullptr)
                {
                    draw_actor_tree_node(a, open_actor_list_popup);
                }
            }

            if(open_actor_list_popup)
            {
                ImGui::OpenPopup(actor_popup_id);
            }

            if (ImGui::BeginPopup(actor_popup_id))
            {
                if (ImGui::Selectable("Remove"))
                {
                    usize remove_index = 0;
                    for(usize i = 0; i < s->actors.size(); ++i)
                    {
                        if(m_editing_actor_guid == s->actors[i].guid)
                        {
                            remove_index = i;
                            break;
                        }
                    }
                    on_remove_actor(m_editing_actor_guid);
                    s->actors.erase(s->actors.begin() + remove_index);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }

        ImGui::EndChild();
        ImGui::PopStyleVar();
    }

    void SceneEditor::draw_actor_tree_node(Actor* actor, bool& open_popup)
    {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
        ActorInfo* info = actor->get_actor_info();
        if(info->get_guid() == m_editing_actor_guid)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        String name(info->name);
        Guid guid = info->get_guid();
        usize hash = memhash(&guid, sizeof(Guid));
        Vector<Actor*> children;
        info->get_children(children);
        if(children.empty())
        {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }
        else
        {
            flags |= ImGuiTreeNodeFlags_DefaultOpen;
        }
        bool opened = ImGui::TreeNodeEx((void*)hash, flags, "%s", name.c_str());
        if(ImGui::IsItemClicked())
        {
            m_editing_actor_guid = info->get_guid();
        }
        if(ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            m_editing_actor_guid = info->get_guid();
            open_popup = true;
        }
        if(ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload("Actor Ref", &m_editing_actor_guid, sizeof(Guid));
            Actor* selected = m_world.get_actor(m_editing_actor_guid);
            ImGui::Text("%s", selected->get_actor_info()->name.c_str());
            ImGui::EndDragDropSource();
        }
        // if(ImGui::BeginDragDropTarget())
        // {
        //     // Actor ref.
        //     {
        //         const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Actor Ref");
        //         if(payload)
        //         {
        //             const Guid* actor_guid = (const Guid*)payload->Data;
        //             Actor* root_actor = actor;
        //             while(root_actor->parent)
        //             {
        //                 root_actor = root_actor->parent;
        //             }
        //             Actor* move_actor = root_actor->get_actor_by_guid(*actor_guid);
        //             if(move_actor)
        //             {
        //                 on_drop_actor(move_actor, actor);
        //             }
        //         }
        //     }
        //     ImGui::EndDragDropTarget();
        // }
        if(opened)
        {
            for(Actor* child : children)
            {
                draw_actor_tree_node(child, open_popup);
            }
            ImGui::TreePop();
        }
    }

    void SceneEditor::draw_scene_settings()
    {
        auto s = get_asset_or_async_load_if_not_ready<Scene>(m_scene);
        ImGui::Text("Scene Settings");

        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
        if(ImGui::BeginChild("Scene Settings", Float2(0.0f, 0.0f), true))
        {
            edit_scene_object(&m_world, typeof<SceneSettings>(), &(s->settings));
        }
        ImGui::EndChild();

        ImGui::PopStyleVar();
    }

    void SceneEditor::draw_scene()
    {
        lutry
        {
            ImGui::Text("Scene");

            Scene* s = get_asset_or_async_load_if_not_ready<Scene>(m_scene);

            m_renderer.world = &m_world;
            Actor* camera_actor = m_world.get_actor(s->settings.camera_actor.guid);
            if(!camera_actor)
            {
                ImGui::Text("Set a camera in scene settings to start.");
                return;
            }
            Camera* camera_component = camera_actor->get_component<Camera>();
            if(!camera_component)
            {
                ImGui::Text("Actor camera actor does not have a camera component");
                return;
            }

            auto& renderer_settings = m_renderer.get_settings();
            camera_component->aspect_ratio = (f32)renderer_settings.screen_size.x / (f32)renderer_settings.screen_size.y;

            // Collect last frame profiling data.
            if(m_renderer.get_settings().frame_profiling)
            {
                m_renderer.collect_frame_profiling_data();
            }

            auto& params = m_renderer.params;
            params.world_to_view = camera_actor->get_world_to_local_matrix();
            params.view_to_world = camera_actor->get_local_to_world_matrix();
            params.view_to_proj = camera_component->get_projection_matrix();
            params.skybox = get_asset_or_async_load_if_not_ready<RHI::ITexture>(s->settings.skybox);
            params.camera_exposure = s->settings.exposure;
            params.camera_fov = camera_component->fov;
            params.camera_type = camera_component->type;
            params.bloom_intensity = s->settings.bloom_intensity;
            params.bloom_threshold = s->settings.bloom_threshold;
            params.camera_auto_exposure = s->settings.auto_exposure;

            luexp(m_renderer.render());

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

            auto& scene_settings = s->settings;

            // Draw GUI Overlays.
            {
                // Draw gizmo.
                Actor* actor = m_world.get_actor(m_editing_actor_guid);
                if (actor && actor != camera_actor)
                {
                    Float4x4 world_mat = actor->get_local_to_world_matrix();
                    bool edited = false;
                    ImGui::Gizmo(world_mat, camera_actor->get_world_to_local_matrix(), camera_component->get_projection_matrix(),
                        RectF(scene_pos.x, scene_pos.y, scene_sz.x, scene_sz.y), m_gizmo_op, m_gizmo_mode, 0.0f, true, false, nullptr, nullptr, &edited);
                    if (edited)
                    {
                        actor->set_local_to_world_matrix(world_mat);
                        // Write transform back to scene data.
                        const Transform* transform = actor->get_transform();
                        s->get_actor(m_editing_actor_guid)->transform = *transform;
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
                if(camera_actor)
                {
                    f32 gizmo_size = 50.0f;
                    f32 gizmo_len = gizmo_size;

                    Transform* camera_transform = camera_actor->get_transform();
                    Float4x4 view_mat = inverse(AffineMatrix::make(camera_transform->position, camera_transform->rotation, Float3(1.0f)));
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
                Transform* camera_transform = camera_actor->get_transform();
                auto rot = camera_transform->rotation;
                auto rot_mat = AffineMatrix::make_rotation(rot);

                // Key control.
                auto left = AffineMatrix::left(rot_mat);
                auto forward = AffineMatrix::forward(rot_mat);
                auto up = AffineMatrix::up(rot_mat);

                f32 camera_speed = m_camera_speed;
                auto& io = ImGui::GetIO();
                if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
                {
                    camera_speed *= 2.0f;
                }

                if (ImGui::IsKeyDown(ImGuiKey_W))
                {
                    camera_transform->position += forward * 0.1f * camera_speed;
                }
                if (ImGui::IsKeyDown(ImGuiKey_A))
                {
                    camera_transform->position += + left * 0.1f * camera_speed;
                }
                if (ImGui::IsKeyDown(ImGuiKey_S))
                {
                    camera_transform->position += - forward * 0.1f * camera_speed;
                }
                if (ImGui::IsKeyDown(ImGuiKey_D))
                {
                    camera_transform->position += - left * 0.1f * camera_speed;
                }
                if (ImGui::IsKeyDown(ImGuiKey_Q))
                {
                    camera_transform->position += - up * 0.1f * camera_speed;
                }
                if (ImGui::IsKeyDown(ImGuiKey_E))
                {
                    camera_transform->position += + up * 0.1f * camera_speed;
                }
                auto eular = AffineMatrix::euler_angles(rot_mat);
                eular += {deg_to_rad((f32)mouse_delta.y / 10.0f), deg_to_rad((f32)mouse_delta.x / 10.0f), 0.0f};
                eular.x = clamp(eular.x, deg_to_rad(-85.0f), deg_to_rad(85.0f));
                camera_transform->rotation = Quaternion::from_euler_angles(eular);

                // Write camera transform back to scene.
                for(auto& scene_actor : s->actors)
                {
                    if(scene_actor.guid == s->settings.camera_actor.guid)
                    {
                        scene_actor.transform = *camera_transform;
                        break;
                    }
                }
            }
            m_renderer.command_buffer->wait();
            luassert_always(succeeded(m_renderer.command_buffer->reset()));
            if(settings != m_renderer.get_settings())
            {
                luexp(m_renderer.reset(settings));
            }
            ImGui::Dummy(ImVec2(0, 0));
            ImGui::EndChild();
        }
        lucatch
        {
            ImGui::Text("%s", explain(luerr));
        }
    }

    static bool edit_transform(Transform* t)
    {
        bool edited = false;

        edited = edited || ImGui::DragFloat3("Position", t->position.m, 0.01f);

        auto euler = AffineMatrix::euler_angles(AffineMatrix::make_rotation(t->rotation));
        euler *= 180.0f / PI;
        if (euler.x > 89.0f || euler.x < -89.0f)
        {
            euler.z = 0.0f;
        }
        ImGui::DragFloat3("Rotation", euler.m);
        if (ImGui::IsItemEdited())
        {
            edited = true;
            euler *= PI / 180.0f;
            t->rotation = Quaternion::from_euler_angles(euler);
        }

        edited = edited || ImGui::DragFloat3("Scale", t->scale.m, 0.01f);

        return edited;
    }

    void SceneEditor::draw_components_grid()
    {
        // Draw component property grid.

        ImGui::Text("Components Grid");

        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
        ImGui::BeginChild("Components Grid", Float2(0.0f, 0.0f), true);

        Scene* s = get_asset_or_async_load_if_not_ready<Scene>(m_scene);
        if(!s) return;

        SceneActor* actor = s->get_actor(m_editing_actor_guid);

        if (actor)
        {
            // Draw name.
            String name = actor->name;
            if(ImGui::InputText("Name", name))
            {
                actor->name = name;
                on_edit_actor_info(*actor);
            }
            // Draw transform.
            if(edit_transform(&actor->transform))
            {
                on_edit_actor_transform(*actor);
            }

            auto& components = actor->components;

            if (components.empty())
            {
                ImGui::Text("No components");
            }
            else
            {
                auto iter = components.begin();

                while (iter != components.end())
                {
                    auto& obj = *iter;
                    if (ImGui::CollapsingHeader(get_type_name(obj.type()).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        bool edited = edit_scene_object(&m_world, obj.type(), obj.get());
                        if(edited)
                        {
                            on_actor_edit_component(*actor, obj.type());
                        }
                        ImGui::PushID(obj.type());

                        if (ImGui::Button("Remove"))
                        {
                            on_actor_remove_component(*actor, obj.type());
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

            const char* new_comp_popup = "NewCompPopup";

            if (ImGui::Button("New Component"))
            {
                ImGui::OpenPopup(new_comp_popup);
            }

            if (ImGui::BeginPopup(new_comp_popup))
            {
                for (auto& i : g_env->component_types)
                {
                    auto exists = false;
                    for(auto& c : components)
                    {
                        if(c.type() == i)
                        {
                            exists = true;
                            break;
                        }
                    }
                    auto name = get_type_name(i);
                    if (!exists)
                    {
                        // Show enabled.
                        if (ImGui::Selectable(name.c_str()))
                        {
                            object_t comp = object_alloc(i);
                            construct_type(i, comp);
                            ObjRef comp_obj;
                            comp_obj.attach(comp);
                            components.push_back(move(comp_obj));
                            on_actor_add_component(*actor, i);
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
        if(!m_world_initialized)
        {
            s->add_to_world(&m_world);
            m_world_initialized = true;
        }
        if (Asset::get_asset_state(m_scene) == Asset::AssetState::unloaded)
        {
            auto _ = Asset::load_asset(m_scene);
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

        draw_actor_list();

        draw_scene_settings();

        ImGui::NextColumn();

        draw_scene();

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
            auto reader = RHIUtility::new_resource_read_context(g_env->device);
            usize op = reader->read_texture(m_renderer.render_texture, SubresourceIndex(0, 0), 0, 0, 0, desc.width, desc.height, 1);
            luexp(reader->commit(readback_cmdbuf, true));
            u32 src_row_pitch, src_slice_pitch;
            lulet(mapped, reader->get_texture_data(op, src_row_pitch, src_slice_pitch));
            memcpy_bitmap(img_data.data(), mapped, row_pitch, desc.height, row_pitch, src_row_pitch);
            Image::ImageDesc img_desc;
            img_desc.width = (u32)desc.width;
            img_desc.height = desc.height;
            img_desc.format = Image::rhi_to_image_format(desc.format);
            luassert(img_desc.format != Image::ImageFormat::unknown);
            lulet(f, open_file(path.encode().c_str(), FileOpenFlag::write | FileOpenFlag::user_buffering, FileCreationMode::create_always));
            luexp(Image::write_bmp_file(f, img_desc, img_data.data(), img_data.size()));
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
                ps_desc.vs = LUNA_GET_SHADER_DATA(GridVS);
                ps_desc.ps = LUNA_GET_SHADER_DATA(GridPS);
                ps_desc.num_color_attachments = 1;
                ps_desc.color_formats[0] = Format::rgba8_unorm;

                luset(m_grid_pso, device->new_graphics_pipeline_state(ps_desc));
            }

            // Upload grid vertex data.
            lulet(upload_cmdbuf, device->new_command_buffer(g_env->async_copy_queue));
            auto writer = RHIUtility::new_resource_write_context(g_env->device);
            lulet(mapped, writer->write_buffer(m_grid_vb, 0, sizeof(grids)));
            memcpy(mapped, grids, sizeof(grids));
            luexp(writer->commit(upload_cmdbuf, true));
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
