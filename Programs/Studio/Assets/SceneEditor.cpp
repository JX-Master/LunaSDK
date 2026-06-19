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
#include "SceneEditorTypes.hpp"
#include "../SceneRenderer.hpp"
#include "../MainEditor.hpp"
#include "../Scene.hpp"
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/Window/MessageBox.hpp>
#include "../EditObject.hpp"
#include "../SceneSettings.hpp"
#include "../Camera.hpp"
#include <Luna/Window/FileDialog.hpp>
#include <Luna/Window/MessageBox.hpp>
#include <Luna/RHIUtility/ResourceReadContext.hpp>
#include <Luna/RHIUtility/ResourceWriteContext.hpp>
#include <Luna/Image/RHIHelper.hpp>
#include <Luna/Runtime/Random.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include "../World.hpp"

#include <GridVS.hpp>
#include <GridPS.hpp>

namespace Luna
{
    namespace
    {
        GUICore::LayoutInput core_fixed_height(f32 height)
        {
            GUICore::LayoutInput layout;
            layout.width.kind = GUICore::SizeKind::expand;
            layout.height.kind = GUICore::SizeKind::pixels;
            layout.height.value = height;
            return layout;
        }

        GUICore::LayoutInput core_fixed_size(f32 width, f32 height)
        {
            GUICore::LayoutInput layout;
            layout.width.kind = GUICore::SizeKind::pixels;
            layout.width.value = width;
            layout.height.kind = GUICore::SizeKind::pixels;
            layout.height.value = height;
            return layout;
        }

        GUICore::LayoutInput core_fixed_width(f32 width)
        {
            GUICore::LayoutInput layout;
            layout.width.kind = GUICore::SizeKind::pixels;
            layout.width.value = width;
            layout.height.kind = GUICore::SizeKind::expand;
            return layout;
        }

        GUICore::LayoutInput core_fill()
        {
            GUICore::LayoutInput layout;
            layout.width.kind = GUICore::SizeKind::expand;
            layout.height.kind = GUICore::SizeKind::expand;
            return layout;
        }

        GUICore::LayoutInput core_ratio_height(f32 ratio)
        {
            GUICore::LayoutInput layout;
            layout.width.kind = GUICore::SizeKind::expand;
            layout.height.kind = GUICore::SizeKind::ratio;
            layout.height.value = ratio;
            return layout;
        }

        GUICore::LinearLayoutDesc core_linear(GUICore::LayoutAxis axis, f32 gap = 6.0f, bool clip_children = false)
        {
            GUICore::LinearLayoutDesc desc;
            desc.axis = axis;
            desc.gap = gap;
            desc.clip_children = clip_children;
            return desc;
        }

        GUICore::ElementHandle core_property_row(GUICore::IContext* context, const c8* label, f32 height = 30.0f)
        {
            GUICore::id_t row_id = context->make_id(label ? label : "row");
            GUICore::ElementHandle row = GUI::begin_h_layout(context, row_id, label, core_fixed_height(height));
            context->push_data_scope(row_id);
            GUI::text(context, context->make_id("label"), label, core_fixed_size(110.0f, height));
            return row;
        }

        void core_end_property_row(GUICore::IContext* context, const GUICore::ElementHandle& row)
        {
            lupanic_if_failed(GUI::end_h_layout(context, row, core_linear(GUICore::LayoutAxis::x, 6.0f)));
            context->pop_data_scope();
        }

        template <typename _Ty>
        _Ty& get_scene_edit_buffer(HashMap<usize, _Ty>& buffers, usize key, const _Ty& default_value)
        {
            auto iter = buffers.find(key);
            if(iter == buffers.end())
            {
                iter = buffers.insert_or_assign(key, default_value).first;
            }
            return iter->second;
        }

        Float3 transform_rotation_to_euler_degrees(const Transform* transform)
        {
            Float3 euler = AffineMatrix::euler_angles(AffineMatrix::make_rotation(transform->rotation));
            euler *= 180.0f / PI;
            if (euler.x > 89.0f || euler.x < -89.0f)
            {
                euler.z = 0.0f;
            }
            return euler;
        }

        HashMap<usize, Float3> g_transform_rotation_edit_buffers;

        bool scene_float3_equal(const Float3& lhs, const Float3& rhs)
        {
            return abs(lhs.x - rhs.x) < 0.00001f && abs(lhs.y - rhs.y) < 0.00001f && abs(lhs.z - rhs.z) < 0.00001f;
        }

        bool edit_transform_core(GUICore::IContext* context, Transform* transform)
        {
            bool edited = false;
            {
                Float3 old_position = transform->position;
                GUICore::ElementHandle row = core_property_row(context, "Position");
                GUI::drag_float3(context, context->make_id("value"), transform->position.m, 0.01f, 0.0f, 0.0f,
                    core_fixed_height(30.0f));
                core_end_property_row(context, row);
                edited |= !scene_float3_equal(transform->position, old_position);
            }
            {
                Float3& euler = get_scene_edit_buffer(g_transform_rotation_edit_buffers, (usize)transform,
                    transform_rotation_to_euler_degrees(transform));
                Float3 old_euler = euler;
                GUICore::ElementHandle row = core_property_row(context, "Rotation");
                GUICore::ElementHandle item = GUI::drag_float3(context, context->make_id("value"), euler.m, 0.1f,
                    0.0f, 0.0f, core_fixed_height(30.0f));
                core_end_property_row(context, row);
                if(!scene_float3_equal(euler, old_euler))
                {
                    Float3 radians = euler * (PI / 180.0f);
                    transform->rotation = Quaternion::from_euler_angles(radians);
                    edited = true;
                }
                else if(!GUI::is_item_active(context, item) && !GUI::is_item_focused(context, item))
                {
                    euler = transform_rotation_to_euler_degrees(transform);
                }
            }
            {
                Float3 old_scale = transform->scale;
                GUICore::ElementHandle row = core_property_row(context, "Scale");
                GUI::drag_float3(context, context->make_id("value"), transform->scale.m, 0.01f, 0.0f, 0.0f,
                    core_fixed_height(30.0f));
                core_end_property_row(context, row);
                edited |= !scene_float3_equal(transform->scale, old_scale);
            }
            return edited;
        }
    }
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

    namespace
    {
        void draw_actor_tree_node_core(SceneEditor* editor, GUICore::IContext* context, Actor* actor,
            bool& open_popup, u32 indent_depth)
        {
            ActorInfo* info = actor->get_actor_info();
            GUI::TreeNodeFlag flags = GUI::TreeNodeFlag::open_on_arrow;
            if(info->get_guid() == editor->m_editing_actor_guid)
            {
                flags |= GUI::TreeNodeFlag::selected;
            }
            String name(info->name);
            Guid guid = info->get_guid();
            usize hash = memhash(&guid, sizeof(Guid));
            Vector<Actor*> children;
            info->get_children(children);
            if(children.empty())
            {
                flags |= GUI::TreeNodeFlag::leaf;
            }
            else
            {
                flags |= GUI::TreeNodeFlag::default_open;
            }

            context->push_data_scope(context->make_id((GUICore::id_t)hash));
            GUICore::ElementHandle tree_node;
            bool opened = GUI::tree_node(context, context->make_id("actor"), name.c_str(), flags, indent_depth,
                core_fixed_height(26.0f), &tree_node);
            if(GUI::is_item_clicked(context, tree_node))
            {
                editor->m_editing_actor_guid = guid;
            }
            if(GUI::is_item_right_clicked(context, tree_node))
            {
                editor->m_editing_actor_guid = guid;
                open_popup = true;
            }
            Name actor_ref_payload_type("Actor Ref");
            if(GUI::begin_drag_drop_source(context, tree_node, actor_ref_payload_type))
            {
                GUI::set_drag_drop_payload(context, &guid, sizeof(guid));
                GUI::text(context, context->make_id("drag_preview"), name.c_str(), core_fixed_height(24.0f));
                GUI::end_drag_drop_source(context);
            }
            if(opened)
            {
                for(Actor* child : children)
                {
                    draw_actor_tree_node_core(editor, context, child, open_popup, indent_depth + 1);
                }
            }
            context->pop_data_scope();
        }
    }

    void SceneEditor::draw_actor_tree_node(GUICore::IContext* context, Actor* actor, bool& open_popup)
    {
        draw_actor_tree_node_core(this, context, actor, open_popup, 0);
    }

    void SceneEditor::draw_actor_list(GUICore::IContext* context, const GUICore::LayoutInput& layout)
    {
        auto s = get_asset_or_async_load_if_not_ready<Scene>(m_scene);
        GUICore::ElementHandle root = GUI::begin_v_layout(context, context->make_id("actor_list"), "Actor List", layout);
        GUICore::ElementHandle header = GUI::begin_h_layout(context, context->make_id("header"), "Actor List Header",
            core_fixed_height(30.0f));
        GUI::text(context, context->make_id("title"), "Actor List", core_fixed_size(110.0f, 30.0f));
        GUICore::ElementHandle new_actor_button = GUI::text_button(context, context->make_id("new_actor"), "New Actor",
            core_fixed_size(110.0f, 30.0f));
        lupanic_if_failed(GUI::end_h_layout(context, header, core_linear(GUICore::LayoutAxis::x, 6.0f)));
        if(s && GUI::is_item_clicked(context, new_actor_button))
        {
            auto iter = s->actors.emplace_back();
            iter->guid = random_guid();
            iter->name = "New Actor";
            on_add_actor(s->actors.size() - 1);
        }

        GUICore::ElementHandle scroll = GUI::begin_scroll_view(context, context->make_id("scroll"), "Actor List Scroll",
            core_fill());
        GUICore::ElementHandle content = GUI::begin_v_layout(context, context->make_id("content"), "Actor List Content",
            core_fill());
        if(!s || s->actors.empty())
        {
            GUI::text(context, context->make_id("empty"), "No actor in the scene.", core_fixed_height(24.0f));
        }
        else
        {
            bool open_actor_list_popup = false;
            for(auto& actor : s->actors)
            {
                Actor* a = m_world.get_actor(actor.guid);
                if(a && a->get_actor_info()->get_parent() == nullptr)
                {
                    draw_actor_tree_node(context, a, open_actor_list_popup);
                }
            }
            if(open_actor_list_popup)
            {
                m_actor_popup_open = true;
                m_actor_popup_position = context->get_pointer_position();
                GUI::open_popup(context, context->make_id("actor_popup"));
            }
        }
        lupanic_if_failed(GUI::end_v_layout(context, content, core_linear(GUICore::LayoutAxis::y, 0.0f)));
        lupanic_if_failed(GUI::end_scroll_view(context, scroll));

        GUICore::id_t popup_id = context->make_id("actor_popup");
        GUI::PopupDesc popup_desc;
        popup_desc.position = m_actor_popup_position;
        popup_desc.layout = core_fixed_size(150.0f, 42.0f);
        GUICore::ElementHandle popup;
        if(GUI::begin_popup(context, popup_id, popup_desc, &popup))
        {
            m_actor_popup_open = true;
            GUICore::ElementHandle remove_item = GUI::selectable(context, context->make_id("remove"), "Remove",
                false, core_fixed_height(28.0f));
            if(s && GUI::is_item_clicked(context, remove_item))
            {
                usize remove_index = USIZE_MAX;
                for(usize i = 0; i < s->actors.size(); ++i)
                {
                    if(m_editing_actor_guid == s->actors[i].guid)
                    {
                        remove_index = i;
                        break;
                    }
                }
                if(remove_index != USIZE_MAX)
                {
                    on_remove_actor(m_editing_actor_guid);
                    s->actors.erase(s->actors.begin() + remove_index);
                }
                m_actor_popup_open = false;
                GUI::close_popup(context, popup_id);
            }
            lupanic_if_failed(GUI::end_popup(context, popup, RectF(0.0f, 0.0f, 150.0f, 42.0f)));
        }
        else if(m_actor_popup_open && !GUI::is_popup_open(context, popup_id))
        {
            m_actor_popup_open = false;
        }

        lupanic_if_failed(GUI::end_v_layout(context, root, core_linear(GUICore::LayoutAxis::y, 6.0f)));
    }

    void SceneEditor::draw_scene_settings(GUICore::IContext* context, const GUICore::LayoutInput& layout)
    {
        auto s = get_asset_or_async_load_if_not_ready<Scene>(m_scene);
        GUICore::ElementHandle root = GUI::begin_v_layout(context, context->make_id("scene_settings"), "Scene Settings",
            layout);
        GUI::text(context, context->make_id("title"), "Scene Settings", core_fixed_height(24.0f));
        GUICore::ElementHandle scroll = GUI::begin_scroll_view(context, context->make_id("scroll"),
            "Scene Settings Scroll", core_fill());
        GUICore::ElementHandle content = GUI::begin_v_layout(context, context->make_id("content"),
            "Scene Settings Content", core_fill());
        if(!s)
        {
            GUI::text(context, context->make_id("loading"), "Scene Loading", core_fixed_height(24.0f));
        }
        else
        {
            edit_scene_object(context, &m_world, typeof<SceneSettings>(), &(s->settings));
        }
        lupanic_if_failed(GUI::end_v_layout(context, content, core_linear(GUICore::LayoutAxis::y, 4.0f)));
        lupanic_if_failed(GUI::end_scroll_view(context, scroll));
        lupanic_if_failed(GUI::end_v_layout(context, root, core_linear(GUICore::LayoutAxis::y, 6.0f)));
    }

    void SceneEditor::draw_scene(GUICore::IContext* context, const GUICore::LayoutInput& layout)
    {
        lutry
        {
            Scene* s = get_asset_or_async_load_if_not_ready<Scene>(m_scene);
            GUICore::ElementHandle root = GUI::begin_v_layout(context, context->make_id("scene_view"), "Scene View", layout);
            GUI::text(context, context->make_id("title"), "Scene", core_fixed_height(24.0f));

            auto settings = m_renderer.get_settings();
            GUICore::FrameDesc frame = context->get_frame_desc();
            f32 dpi_scale = max(frame.dpi_scale, 1.0f);
            UInt2U target_size(
                (u32)max(frame.screen_size.x * 0.5f * dpi_scale, 64.0f),
                (u32)max(frame.screen_size.y * 0.55f * dpi_scale, 64.0f));
            settings.screen_size = target_size;

            if(settings != m_renderer.get_settings())
            {
                luexp(m_renderer.reset(settings));
            }

            GUICore::ElementHandle toolbar = GUI::begin_h_layout(context, context->make_id("toolbar"), "Scene Toolbar",
                core_fixed_height(32.0f));
            GUI::slider_float(context, context->make_id("camera_speed"), &m_camera_speed, 0.1f, 10.0f,
                core_fixed_size(190.0f, 32.0f));
            GUI::text(context, context->make_id("mode_label"), "Mode", core_fixed_size(48.0f, 32.0f));
            GUICore::ElementHandle local_mode = GUI::selectable(context, context->make_id("local_mode"), "Local",
                m_gizmo_mode == GUI::GizmoMode::local, core_fixed_size(64.0f, 32.0f));
            GUICore::ElementHandle world_mode = GUI::selectable(context, context->make_id("world_mode"), "World",
                m_gizmo_mode == GUI::GizmoMode::world, core_fixed_size(72.0f, 32.0f));
            GUI::text(context, context->make_id("operation_label"), "Operation", core_fixed_size(78.0f, 32.0f));
            GUICore::ElementHandle translate_op = GUI::selectable(context, context->make_id("translate"), "Translate",
                m_gizmo_op == GUI::GizmoOperation::translate, core_fixed_size(90.0f, 32.0f));
            GUICore::ElementHandle rotate_op = GUI::selectable(context, context->make_id("rotate"), "Rotate",
                m_gizmo_op == GUI::GizmoOperation::rotate, core_fixed_size(72.0f, 32.0f));
            GUICore::ElementHandle scale_op = GUI::selectable(context, context->make_id("scale"), "Scale",
                m_gizmo_op == GUI::GizmoOperation::scale, core_fixed_size(64.0f, 32.0f));

            auto render_mode_type = typeof<SceneRendererMode>();
            auto options = get_enum_options(render_mode_type);
            Name current_name;
            usize current_mode_index = 0;
            for(usize i = 0; i < options.size(); ++i)
            {
                if(options[i].value == (i64)settings.mode)
                {
                    current_name = options[i].name;
                    current_mode_index = i;
                    break;
                }
            }
            if(current_name.empty() && !options.empty())
            {
                current_name = options[0].name;
            }
            String render_mode_label;
            strprintf(render_mode_label, "Render: %s", current_name.c_str());
            GUICore::ElementHandle render_mode_button = GUI::text_button(context, context->make_id("render_mode"),
                render_mode_label.c_str(), core_fixed_size(170.0f, 32.0f));
            GUICore::ElementHandle profiling_button = GUI::selectable(context, context->make_id("profiling"),
                "Time Profiling", settings.frame_profiling, core_fixed_size(140.0f, 32.0f));
            lupanic_if_failed(GUI::end_h_layout(context, toolbar, core_linear(GUICore::LayoutAxis::x, 6.0f)));

            if(GUI::is_item_clicked(context, local_mode)) m_gizmo_mode = GUI::GizmoMode::local;
            if(GUI::is_item_clicked(context, world_mode)) m_gizmo_mode = GUI::GizmoMode::world;
            if(GUI::is_item_clicked(context, translate_op)) m_gizmo_op = GUI::GizmoOperation::translate;
            if(GUI::is_item_clicked(context, rotate_op)) m_gizmo_op = GUI::GizmoOperation::rotate;
            if(GUI::is_item_clicked(context, scale_op)) m_gizmo_op = GUI::GizmoOperation::scale;
            if(GUI::is_item_clicked(context, render_mode_button) && !options.empty())
            {
                settings.mode = (SceneRendererMode)options[(current_mode_index + 1) % options.size()].value;
            }
            if(GUI::is_item_clicked(context, profiling_button))
            {
                settings.frame_profiling = !settings.frame_profiling;
            }
            if(settings != m_renderer.get_settings())
            {
                luexp(m_renderer.reset(settings));
            }

            GUICore::ElementHandle viewport = GUI::begin_stack_layout(context, context->make_id("viewport"), "Scene Viewport",
                core_fill());
            GUICore::ElementHandle viewport_hit = GUI::hit_box(context, context->make_id("viewport_hit"), core_fill());

            if(!s)
            {
                GUI::text(context, context->make_id("loading"), "Scene Loading", core_fill());
            }
            else
            {
                m_renderer.world = &m_world;
                Actor* camera_actor = m_world.get_actor(s->settings.camera_actor.guid);
                Camera* camera_component = camera_actor ? camera_actor->get_component<Camera>() : nullptr;
                if(!camera_actor)
                {
                    GUI::text(context, context->make_id("no_camera"), "Set a camera in scene settings to start.", core_fill());
                }
                else if(!camera_component)
                {
                    GUI::text(context, context->make_id("no_camera_component"),
                        "Actor camera actor does not have a camera component", core_fill());
                }
                else
                {
                    const SceneRendererSettings& renderer_settings = m_renderer.get_settings();
                    camera_component->aspect_ratio = (f32)renderer_settings.screen_size.x / (f32)renderer_settings.screen_size.y;

                    if(renderer_settings.frame_profiling)
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
                    luexp(m_renderer.command_buffer->submit({}, {}, true));
                    m_renderer.command_buffer->wait();
                    luassert_always(succeeded(m_renderer.command_buffer->reset()));

                    GUI::image(context, context->make_id("scene_texture"), m_renderer.render_texture.get(),
                        core_fill(), GUI::ImageFlag::flip_y);

                    bool scene_pointer_hovered = GUI::is_item_hovered(context, viewport_hit);
                    bool right_down = context->is_pointer_button_down(GUICore::PointerButton::right);
                    if(!m_navigating && right_down && scene_pointer_hovered)
                    {
                        m_navigating = true;
                        m_scene_pointer_initialized = false;
                    }
                    if(m_navigating && !right_down)
                    {
                        m_navigating = false;
                    }
                    if(m_navigating)
                    {
                        Float2U pointer = context->get_pointer_position();
                        Float2U mouse_delta(0.0f);
                        if(m_scene_pointer_initialized)
                        {
                            mouse_delta.x = pointer.x - m_last_scene_pointer.x;
                            mouse_delta.y = pointer.y - m_last_scene_pointer.y;
                        }
                        m_last_scene_pointer = pointer;
                        m_scene_pointer_initialized = true;

                        Transform* camera_transform = camera_actor->get_transform();
                        auto rot = camera_transform->rotation;
                        auto rot_mat = AffineMatrix::make_rotation(rot);
                        auto left = AffineMatrix::left(rot_mat);
                        auto forward = AffineMatrix::forward(rot_mat);
                        auto up = AffineMatrix::up(rot_mat);

                        f32 camera_speed = m_camera_speed;
                        if(((u8)context->get_key_modifiers() & (u8)GUICore::KeyModifierFlag::shift) != 0)
                        {
                            camera_speed *= 2.0f;
                        }
                        if(context->is_key_down(KeyCode::w))
                        {
                            camera_transform->position += forward * 0.1f * camera_speed;
                        }
                        if(context->is_key_down(KeyCode::a))
                        {
                            camera_transform->position += +left * 0.1f * camera_speed;
                        }
                        if(context->is_key_down(KeyCode::s))
                        {
                            camera_transform->position += -forward * 0.1f * camera_speed;
                        }
                        if(context->is_key_down(KeyCode::d))
                        {
                            camera_transform->position += -left * 0.1f * camera_speed;
                        }
                        if(context->is_key_down(KeyCode::q))
                        {
                            camera_transform->position += -up * 0.1f * camera_speed;
                        }
                        if(context->is_key_down(KeyCode::e))
                        {
                            camera_transform->position += +up * 0.1f * camera_speed;
                        }
                        auto eular = AffineMatrix::euler_angles(rot_mat);
                        eular += {deg_to_rad((f32)mouse_delta.y / 10.0f), deg_to_rad((f32)mouse_delta.x / 10.0f), 0.0f};
                        eular.x = clamp(eular.x, deg_to_rad(-85.0f), deg_to_rad(85.0f));
                        camera_transform->rotation = Quaternion::from_euler_angles(eular);

                        for(auto& scene_actor : s->actors)
                        {
                            if(scene_actor.guid == s->settings.camera_actor.guid)
                            {
                                scene_actor.transform = *camera_transform;
                                break;
                            }
                        }
                    }
                    else
                    {
                        m_scene_pointer_initialized = false;
                    }

                    if(renderer_settings.frame_profiling)
                    {
                        String debug_text;
                        strprintf(debug_text, "Frame Size: %ux%u", renderer_settings.screen_size.x, renderer_settings.screen_size.y);
                        GUI::text(context, context->make_id("frame_size"), debug_text.c_str(), core_fixed_height(22.0f));
                        f32 fps = frame.delta_time > 0.0f ? 1.0f / frame.delta_time : 0.0f;
                        strprintf(debug_text, "FPS: %f", fps);
                        GUI::text(context, context->make_id("fps"), debug_text.c_str(), core_fixed_height(22.0f));
                    }
                }
            }

            lupanic_if_failed(GUI::end_stack_layout(context, viewport, GUICore::StackLayoutDesc()));
            lupanic_if_failed(GUI::end_v_layout(context, root, core_linear(GUICore::LayoutAxis::y, 6.0f)));
        }
        lucatch
        {
            GUICore::ElementHandle root = GUI::begin_v_layout(context, context->make_id("scene_view_error"), "Scene View Error",
                layout);
            GUI::text(context, context->make_id("error"), explain(luerr), core_fixed_height(30.0f));
            lupanic_if_failed(GUI::end_v_layout(context, root, core_linear(GUICore::LayoutAxis::y, 6.0f)));
        }
    }

    void SceneEditor::draw_components_grid(GUICore::IContext* context, const GUICore::LayoutInput& layout)
    {
        GUICore::ElementHandle root = GUI::begin_v_layout(context, context->make_id("components_grid"), "Components Grid",
            layout);
        GUI::text(context, context->make_id("title"), "Components Grid", core_fixed_height(24.0f));
        GUICore::ElementHandle scroll = GUI::begin_scroll_view(context, context->make_id("scroll"),
            "Components Grid Scroll", core_fill());
        GUICore::ElementHandle content = GUI::begin_v_layout(context, context->make_id("content"),
            "Components Grid Content", core_fill());

        Scene* s = get_asset_or_async_load_if_not_ready<Scene>(m_scene);
        if(!s)
        {
            GUI::text(context, context->make_id("loading"), "Scene Loading", core_fixed_height(24.0f));
        }
        else
        {
            SceneActor* actor = s->get_actor(m_editing_actor_guid);
            if(actor)
            {
                if(m_actor_name_editing_guid != actor->guid)
                {
                    m_actor_name_editing_guid = actor->guid;
                    m_actor_name_editing_text = actor->name.c_str();
                }
                {
                    String old_name = actor->name.c_str();
                    GUICore::ElementHandle row = core_property_row(context, "Actor Name");
                    GUI::input_text(context, context->make_id("name"), m_actor_name_editing_text, core_fixed_height(30.0f));
                    core_end_property_row(context, row);
                    if(strcmp(m_actor_name_editing_text.c_str(), old_name.c_str()) != 0)
                    {
                        actor->name = m_actor_name_editing_text;
                        on_edit_actor_info(*actor);
                    }
                }

                if(edit_transform_core(context, &actor->transform))
                {
                    on_edit_actor_transform(*actor);
                }

                auto& components = actor->components;
                if(components.empty())
                {
                    GUI::text(context, context->make_id("no_components"), "No components", core_fixed_height(24.0f));
                }
                else
                {
                    auto iter = components.begin();
                    while(iter != components.end())
                    {
                        auto& obj = *iter;
                        Name type_name = get_type_name(obj.type());
                        context->push_data_scope(context->make_id((GUICore::id_t)obj.type().handle));
                        GUICore::ElementHandle header_handle;
                        bool open = GUI::collapsing_header(context, context->make_id("component_header"),
                            type_name.c_str(), true, core_fixed_height(30.0f), &header_handle);
                        bool remove_component = false;
                        if(open)
                        {
                            bool edited = edit_scene_object(context, &m_world, obj.type(), obj.get());
                            if(edited)
                            {
                                on_actor_edit_component(*actor, obj.type());
                            }
                            GUICore::ElementHandle remove_button = GUI::text_button(context, context->make_id("remove"),
                                "Remove", core_fixed_height(28.0f));
                            remove_component = GUI::is_item_clicked(context, remove_button);
                        }
                        context->pop_data_scope();

                        if(remove_component)
                        {
                            on_actor_remove_component(*actor, obj.type());
                            iter = components.erase(iter);
                        }
                        else
                        {
                            ++iter;
                        }
                    }
                }

                GUICore::ElementHandle new_component_button = GUI::text_button(context, context->make_id("new_component"),
                    "New Component", core_fixed_height(30.0f));
                GUICore::id_t popup_id = context->make_id("new_component_popup");
                if(GUI::is_item_clicked(context, new_component_button))
                {
                    m_new_component_popup_open = true;
                    m_new_component_popup_position = context->get_pointer_position();
                    GUI::open_popup(context, popup_id);
                }
                GUI::PopupDesc popup_desc;
                popup_desc.position = m_new_component_popup_position;
                popup_desc.layout = core_fixed_size(260.0f, max((f32)g_env->component_types.size() * 28.0f + 12.0f, 42.0f));
                GUICore::ElementHandle popup;
                if(GUI::begin_popup(context, popup_id, popup_desc, &popup))
                {
                    Vector<Pair<typeinfo_t, GUICore::ElementHandle>> component_items;
                    component_items.reserve(g_env->component_types.size());
                    for(auto& component_type : g_env->component_types)
                    {
                        bool exists = false;
                        for(auto& component : components)
                        {
                            if(component.type() == component_type)
                            {
                                exists = true;
                                break;
                            }
                        }
                        Name comp_name = get_type_name(component_type);
                        context->push_data_scope(context->make_id((GUICore::id_t)component_type.handle));
                        if(!exists)
                        {
                            component_items.push_back(make_pair(component_type,
                                GUI::selectable(context, context->make_id("component"), comp_name.c_str(), false,
                                    core_fixed_height(26.0f))));
                        }
                        else
                        {
                            GUI::text(context, context->make_id("existing"), comp_name.c_str(), core_fixed_height(26.0f));
                        }
                        context->pop_data_scope();
                    }
                    for(auto& item : component_items)
                    {
                        if(GUI::is_item_clicked(context, item.second))
                        {
                            object_t comp = object_alloc(item.first);
                            construct_type(item.first, comp);
                            ObjRef comp_obj;
                            comp_obj.attach(comp);
                            components.push_back(move(comp_obj));
                            on_actor_add_component(*actor, item.first);
                            m_new_component_popup_open = false;
                            GUI::close_popup(context, popup_id);
                        }
                    }
                    lupanic_if_failed(GUI::end_popup(context, popup,
                        RectF(0.0f, 0.0f, 260.0f, max((f32)g_env->component_types.size() * 28.0f + 12.0f, 42.0f))));
                }
                else if(m_new_component_popup_open && !GUI::is_popup_open(context, popup_id))
                {
                    m_new_component_popup_open = false;
                }
            }
            else
            {
                GUI::text(context, context->make_id("no_actor"), "Select an entity to see components.",
                    core_fixed_height(24.0f));
            }
        }

        lupanic_if_failed(GUI::end_v_layout(context, content, core_linear(GUICore::LayoutAxis::y, 4.0f)));
        lupanic_if_failed(GUI::end_scroll_view(context, scroll));
        lupanic_if_failed(GUI::end_v_layout(context, root, core_linear(GUICore::LayoutAxis::y, 6.0f)));
    }

    void SceneEditor::on_render(GUICore::IContext* context, const GUICore::LayoutInput& layout)
    {
        if(!m_open)
        {
            return;
        }

        context->push_data_scope(context->make_id((GUICore::id_t)(usize)this));
        GUICore::ElementHandle root = GUI::begin_v_layout(context, context->make_id("scene_editor"), "Scene Editor",
            layout);

        auto s = get_asset_or_async_load_if_not_ready<Scene>(m_scene);
        if(!s)
        {
            GUI::text(context, context->make_id("asset_unloaded"), "Asset Unloaded", core_fixed_height(30.0f));
            lupanic_if_failed(GUI::end_v_layout(context, root, core_linear(GUICore::LayoutAxis::y, 6.0f)));
            context->pop_data_scope();
            return;
        }
        if(!m_world_initialized)
        {
            s->add_to_world(&m_world);
            m_world_initialized = true;
        }
        if(Asset::get_asset_state(m_scene) == Asset::AssetState::unloaded)
        {
            auto _ = Asset::load_asset(m_scene);
        }
        if(Asset::get_asset_state(m_scene) != Asset::AssetState::loaded)
        {
            GUI::text(context, context->make_id("scene_loading"), "Scene Loading", core_fixed_height(30.0f));
            lupanic_if_failed(GUI::end_v_layout(context, root, core_linear(GUICore::LayoutAxis::y, 6.0f)));
            context->pop_data_scope();
            return;
        }

        bool capture_scene = false;
        Path capture_save_path;

        GUICore::ElementHandle menu_bar = GUI::begin_menu_bar(context, context->make_id("menu_bar"),
            "Scene Editor Menu Bar", core_fixed_height(30.0f));
        GUICore::ElementHandle save_item;
        if(GUI::begin_menu(context, context->make_id("file"), "File"))
        {
            save_item = GUI::menu_item(context, context->make_id("save"), "Save");
            lupanic_if_failed(GUI::end_menu(context, RectF(0.0f, 0.0f, 180.0f, 34.0f)));
        }
        GUICore::ElementHandle capture_item;
        if(GUI::begin_menu(context, context->make_id("tools"), "Tools"))
        {
            capture_item = GUI::menu_item(context, context->make_id("capture_scene"), "Capture scene");
            lupanic_if_failed(GUI::end_menu(context, RectF(0.0f, 0.0f, 220.0f, 34.0f)));
        }
        lupanic_if_failed(GUI::end_menu_bar(context, menu_bar));

        if(GUI::is_item_clicked(context, save_item))
        {
            lutry
            {
                luexp(Asset::save_asset(m_scene));
            }
            lucatch
            {
                auto _ = Window::message_box(explain(luerr), "Failed to save scene",
                    Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
            }
        }
        if(GUI::is_item_clicked(context, capture_item))
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

        GUICore::ElementHandle content = GUI::begin_h_layout(context, context->make_id("content"), "Scene Editor Content",
            core_fill());
        GUICore::ElementHandle left_column = GUI::begin_v_layout(context, context->make_id("left_column"),
            "Scene Editor Left Column", core_fixed_width(320.0f));
        draw_actor_list(context, core_ratio_height(1.0f));
        draw_scene_settings(context, core_ratio_height(1.0f));
        lupanic_if_failed(GUI::end_v_layout(context, left_column, core_linear(GUICore::LayoutAxis::y, 8.0f)));

        draw_scene(context, core_fill());
        draw_components_grid(context, core_fixed_width(380.0f));
        lupanic_if_failed(GUI::end_h_layout(context, content, core_linear(GUICore::LayoutAxis::x, 8.0f)));
        lupanic_if_failed(GUI::end_v_layout(context, root, core_linear(GUICore::LayoutAxis::y, 6.0f)));
        context->pop_data_scope();

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
                auto attribute = InputAttributeDesc(0, 0, 0, Format::rgba32_float);
                auto binding = InputBindingDesc(0, sizeof(Float4U), InputRate::per_vertex);
                ps_desc.input_layout.attributes = { &attribute, 1 };
                ps_desc.input_layout.bindings = { &binding, 1 };
                ps_desc.pipeline_layout = m_grid_playout;
                ps_desc.vs = LUNA_CPPSL_GET_SHADER_DATA(GridVS);
                ps_desc.ps = LUNA_CPPSL_GET_SHADER_DATA(GridPS);
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
            AssetEditorDesc desc;
            desc.new_editor = new_scene_editor;
            auto userdata = new_object<SceneEditorUserData>();
            luexp(userdata->init());
            desc.userdata = userdata;
            g_env->register_asset_editor_type(get_scene_asset_type(), desc);
        }
        lucatchret;
        return ok;
    }
}
