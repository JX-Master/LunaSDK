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
#include <Luna/GUI/GUI.hpp>
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
        struct SceneGUIRow
        {
            Float2 size;
        };

        u32 g_scene_gui_flow_depth = 0;

        struct SceneGUIFlowScope
        {
            GUI::IContext* context;

            SceneGUIFlowScope(GUI::IContext* ctx) :
                context(ctx)
            {
                ++g_scene_gui_flow_depth;
                push_edit_object_gui_flow_layout(context);
            }

            ~SceneGUIFlowScope()
            {
                pop_edit_object_gui_flow_layout(context);
                luassert(g_scene_gui_flow_depth);
                --g_scene_gui_flow_depth;
            }
        };

        SceneGUIRow begin_scene_gui_row(GUI::IContext* context, const c8* label, f32 height = 30.0f)
        {
            (void)g_scene_gui_flow_depth;
            GUI::LayoutStyle row_style = GUI::LayoutStyle::fill_width();
            row_style.height_policy = GUI::SizePolicy::fixed;
            row_style.fixed_height_value = height;
            GUI::set_next_item_layout(context, row_style);
            GUI::LayoutDesc row;
            row.cross_axis_alignment = GUI::LayoutCrossAxisAlignment::stretch;
            GUI::begin_h_layout(context, label, row);
            GUI::set_next_item_layout(context, GUI::LayoutStyle::fill_width());
            return SceneGUIRow { Float2(0.0f, height) };
        }

        bool end_scene_gui_row(GUI::IContext* context, const SceneGUIRow& row, GUI::ItemHandle item)
        {
            (void)row;
            GUI::end_h_layout(context);
            return GUI::get_item_state(item, GUI::State::value_changed());
        }

        void draw_scene_gui_text_line(GUI::IContext* context, const c8* text, f32 height = 24.0f)
        {
            (void)g_scene_gui_flow_depth;
            GUI::LayoutStyle text_style = GUI::LayoutStyle::fill_width();
            text_style.height_policy = GUI::SizePolicy::fixed;
            text_style.fixed_height_value = height;
            GUI::set_next_item_layout(context, text_style);
            GUI::text(context, text);
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
    }

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
        Guid m_actor_name_editing_guid = Guid(0, 0);
        String m_actor_name_editing_text;
        bool m_actor_popup_open = false;
        Float2U m_actor_popup_position = Float2U(0.0f);
        GUI::ItemHandle m_actor_popup_handle;
        bool m_new_component_popup_open = false;
        Float2U m_new_component_popup_position = Float2U(0.0f);
        GUI::ItemHandle m_new_component_popup_handle;

        // States for scene viewport.

        GUI::GizmoMode m_gizmo_mode = GUI::GizmoMode::local;
        GUI::GizmoOperation m_gizmo_op = GUI::GizmoOperation::translate;

        f32 m_camera_speed = 1.0f;

        bool m_navigating = false;

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
        void draw_actor_list(GUI::IContext* context, const RectF& rect);
        void draw_actor_tree_node(GUI::IContext* context, Actor* actor, bool& open_actor_list_popup);
        void draw_scene_settings(GUI::IContext* context, const RectF& rect);
        void draw_scene(GUI::IContext* context, const RectF& rect);

        void draw_components_grid(GUI::IContext* context, const RectF& rect);

        virtual void on_render(GUI::IContext* context) override;
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

    void SceneEditor::draw_actor_list(GUI::IContext* context, const RectF& rect)
    {
        auto s = get_asset_or_async_load_if_not_ready<Scene>(m_scene);

        // Draw  list.
        constexpr f32 header_height = 30.0f;
        constexpr f32 panel_gap = 6.0f;
        GUI::LayoutDesc header_layout;
        header_layout.cross_axis_alignment = GUI::LayoutCrossAxisAlignment::stretch;
        GUI::begin_h_layout(context, "Actor List Header", RectF(rect.offset_x, rect.offset_y, rect.width, header_height), header_layout);
        GUI::set_next_item_layout(context, GUI::LayoutStyle::fixed_width(96.0f));
        GUI::text(context, "Actor List");
        GUI::set_next_item_layout(context, GUI::LayoutStyle::fixed_width(110.0f));
        GUI::ItemHandle new_actor_button = GUI::button(context, "New Actor");
        GUI::end_h_layout(context);

        if (GUI::is_item_clicked(new_actor_button))
        {
            auto iter = s->actors.emplace_back();
            iter->guid = random_guid();
            iter->name = "New Actor";
            on_add_actor(s->actors.size() - 1);
        }

        RectF list_rect(rect.offset_x, rect.offset_y + header_height + panel_gap, rect.width,
            max(rect.height - header_height - panel_gap, 1.0f));

        GUI::push_id(context, this);
        GUI::draw_rect(context, list_rect, Float4U(0.16f, 0.19f, 0.24f, 1.0f), 5.0f);
        GUI::draw_rect(context, RectF(list_rect.offset_x + 1.0f, list_rect.offset_y + 1.0f, max(list_rect.width - 2.0f, 1.0f), max(list_rect.height - 2.0f, 1.0f)),
            Float4U(0.04f, 0.05f, 0.06f, 1.0f), 4.0f);

        GUI::LayoutDesc list_host_layout;
        list_host_layout.padding = GUI::EdgeInsets::all(1.0f);
        list_host_layout.gap = 0.0f;
        GUI::begin_v_layout(context, "Actor List Host", list_rect, list_host_layout);
        GUI::begin_scroll_view(context, "Actor List", GUI::Size::fixed(max(list_rect.width - 2.0f, 1.0f), max(list_rect.height - 2.0f, 1.0f)));

        if (s->actors.empty())
        {
            GUI::text(context, "No actor in the scene.");
        }
        else
        {
            bool open_actor_list_popup = false;

            for(auto& actor : s->actors)
            {
                Actor* a = m_world.get_actor(actor.guid);
                if(a->get_actor_info()->get_parent() == nullptr)
                {
                    draw_actor_tree_node(context, a, open_actor_list_popup);
                }
            }

            if(open_actor_list_popup)
            {
                m_actor_popup_open = true;
                m_actor_popup_position = GUI::get_pointer_position(context);
                GUI::open_popup(context, m_actor_popup_handle);
            }

            {
                GUI::ItemHandle remove_item;
                bool popup_open = GUI::begin_popup(context, "Actor Popup", m_actor_popup_position, GUI::Size::fixed(150.0f, 42.0f), &m_actor_popup_handle);
                if(popup_open)
                {
                    m_actor_popup_open = true;
                    remove_item = GUI::selectable(context, "Remove");
                    GUI::end_popup(context);
                }
                else if(m_actor_popup_open && !GUI::is_popup_open(context, m_actor_popup_handle))
                {
                    m_actor_popup_open = false;
                }
                if (GUI::is_item_clicked(remove_item))
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
                    m_actor_popup_open = false;
                    GUI::close_popup(context, m_actor_popup_handle);
                }
            }
        }

        GUI::end_scroll_view(context);
        GUI::end_v_layout(context);
        GUI::pop_id(context);
    }

    void SceneEditor::draw_actor_tree_node(GUI::IContext* context, Actor* actor, bool& open_popup)
    {
        ActorInfo* info = actor->get_actor_info();
        GUI::TreeNodeFlag flags = GUI::TreeNodeFlag::open_on_arrow;
        if(info->get_guid() == m_editing_actor_guid)
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
        GUI::push_id(context, (u64)hash);
        GUI::ItemHandle tree_node = GUI::tree_node(context, name.c_str(), flags);
        GUI::pop_id(context);
        if(GUI::is_item_clicked(tree_node))
        {
            m_editing_actor_guid = info->get_guid();
        }
        if(GUI::is_item_right_clicked(tree_node))
        {
            m_editing_actor_guid = info->get_guid();
            open_popup = true;
        }
        bool opened = GUI::get_item_state(tree_node, GUI::State::open());
        if(opened)
        {
            GUI::tree_push(context, tree_node);
            for(Actor* child : children)
            {
                draw_actor_tree_node(context, child, open_popup);
            }
            GUI::tree_pop(context);
        }

        Name actor_ref_payload_type("Actor Ref");
        if(GUI::begin_drag_drop_source(context, tree_node, actor_ref_payload_type))
        {
            GUI::set_drag_drop_payload(context, &guid, sizeof(guid));
            GUI::text(context, name.c_str());
            GUI::end_drag_drop_source(context);
        }

    }

    void SceneEditor::draw_scene_settings(GUI::IContext* context, const RectF& rect)
    {
        auto s = get_asset_or_async_load_if_not_ready<Scene>(m_scene);
        constexpr f32 title_height = 24.0f;
        constexpr f32 panel_gap = 6.0f;
        GUI::draw_text(context, RectF(rect.offset_x, rect.offset_y, rect.width, title_height), "Scene Settings", Color::white(), 16.0f,
            GUI::TextAlignment::begin, GUI::TextAlignment::center);

        RectF panel_rect(rect.offset_x, rect.offset_y + title_height + panel_gap, rect.width,
            max(rect.height - title_height - panel_gap, 1.0f));

        GUI::draw_rect(context, panel_rect, Float4U(0.16f, 0.19f, 0.24f, 1.0f), 5.0f);
        GUI::draw_rect(context, RectF(panel_rect.offset_x + 1.0f, panel_rect.offset_y + 1.0f, max(panel_rect.width - 2.0f, 1.0f), max(panel_rect.height - 2.0f, 1.0f)),
            Float4U(0.04f, 0.05f, 0.06f, 1.0f), 4.0f);

        GUI::LayoutDesc panel_layout;
        panel_layout.padding = GUI::EdgeInsets::all(6.0f);
        panel_layout.gap = 0.0f;
        GUI::begin_v_layout(context, "Scene Settings Panel", panel_rect, panel_layout);
        GUI::begin_scroll_view(context, "Scene Settings Scroll", GUI::Size::fixed(max(panel_rect.width - 12.0f, 1.0f), max(panel_rect.height - 12.0f, 1.0f)));
        {
            SceneGUIFlowScope flow(context);
            edit_scene_object(context, &m_world, typeof<SceneSettings>(), &(s->settings));
        }
        GUI::end_scroll_view(context);
        GUI::end_v_layout(context);
    }

    void SceneEditor::draw_scene(GUI::IContext* context, const RectF& rect)
    {
        lutry
        {
            constexpr f32 title_height = 24.0f;
            constexpr f32 panel_gap = 6.0f;
            GUI::draw_text(context, RectF(rect.offset_x, rect.offset_y, rect.width, title_height), "Scene", Color::white(), 16.0f,
                GUI::TextAlignment::begin, GUI::TextAlignment::center);
            RectF viewport_rect(rect.offset_x, rect.offset_y + title_height + panel_gap, rect.width,
                max(rect.height - title_height - panel_gap, 1.0f));

            Scene* s = get_asset_or_async_load_if_not_ready<Scene>(m_scene);

            m_renderer.world = &m_world;
            Actor* camera_actor = m_world.get_actor(s->settings.camera_actor.guid);
            if(!camera_actor)
            {
                GUI::draw_text(context, viewport_rect, "Set a camera in scene settings to start.", Color::white(), 16.0f,
                    GUI::TextAlignment::begin, GUI::TextAlignment::begin);
                return;
            }
            Camera* camera_component = camera_actor->get_component<Camera>();
            if(!camera_component)
            {
                GUI::draw_text(context, viewport_rect, "Actor camera actor does not have a camera component", Color::white(), 16.0f,
                    GUI::TextAlignment::begin, GUI::TextAlignment::begin);
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

            auto settings = m_renderer.get_settings();
            Float2 viewport_pos(viewport_rect.offset_x, viewport_rect.offset_y);
            Float2 viewport_size(max(viewport_rect.width, 1.0f), max(viewport_rect.height, 1.0f));

            f32 toolbar_height = 32.0f;
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

            GUI::push_id(context, this);
            GUI::begin_canvas_layout(context, "Scene Viewport Canvas", RectF(viewport_pos.x, viewport_pos.y, viewport_size.x, viewport_size.y));
            GUI::CanvasItemLayout toolbar_canvas;
            toolbar_canvas.anchor_min = Float2U(0.0f, 0.0f);
            toolbar_canvas.anchor_max = Float2U(1.0f, 0.0f);
            toolbar_canvas.offset_min = Float2U(0.0f, 0.0f);
            toolbar_canvas.offset_max = Float2U(0.0f, toolbar_height);
            GUI::set_next_canvas_item_layout(context, toolbar_canvas);
            GUI::LayoutDesc toolbar_layout;
            toolbar_layout.gap = 6.0f;
            toolbar_layout.cross_axis_alignment = GUI::LayoutCrossAxisAlignment::stretch;
            GUI::begin_h_layout(context, "Scene Viewport Toolbar", toolbar_layout);
            GUI::set_next_item_layout(context, GUI::LayoutStyle::fixed_width(190.0f));
            GUI::slider_float(context, "Camera Speed", &m_camera_speed, 0.1f, 10.0f);
            GUI::set_next_item_layout(context, GUI::LayoutStyle::fixed_width(84.0f));
            GUI::text(context, "Mode");
            GUI::set_next_item_layout(context, GUI::LayoutStyle::fixed_width(64.0f));
            GUI::ItemHandle local_mode = GUI::selectable(context, "Local", m_gizmo_mode == GUI::GizmoMode::local);
            GUI::set_next_item_layout(context, GUI::LayoutStyle::fixed_width(64.0f));
            GUI::ItemHandle world_mode = GUI::selectable(context, "World", m_gizmo_mode == GUI::GizmoMode::world);
            GUI::set_next_item_layout(context, GUI::LayoutStyle::fixed_width(72.0f));
            GUI::text(context, "Operation");
            GUI::set_next_item_layout(context, GUI::LayoutStyle::fixed_width(90.0f));
            GUI::ItemHandle translate_op = GUI::selectable(context, "Translate", m_gizmo_op == GUI::GizmoOperation::translate);
            GUI::set_next_item_layout(context, GUI::LayoutStyle::fixed_width(72.0f));
            GUI::ItemHandle rotate_op = GUI::selectable(context, "Rotate", m_gizmo_op == GUI::GizmoOperation::rotate);
            GUI::set_next_item_layout(context, GUI::LayoutStyle::fixed_width(64.0f));
            GUI::ItemHandle scale_op = GUI::selectable(context, "Scale", m_gizmo_op == GUI::GizmoOperation::scale);
            GUI::set_next_item_layout(context, GUI::LayoutStyle::fixed_width(170.0f));
            GUI::ItemHandle render_mode_button = GUI::button(context, render_mode_label.c_str());
            GUI::set_next_item_layout(context, GUI::LayoutStyle::fixed_width(140.0f));
            GUI::ItemHandle profiling_button = GUI::selectable(context, "Time Profiling", settings.frame_profiling);
            GUI::end_h_layout(context);

            if(GUI::is_item_clicked(local_mode)) m_gizmo_mode = GUI::GizmoMode::local;
            if(GUI::is_item_clicked(world_mode)) m_gizmo_mode = GUI::GizmoMode::world;
            if(GUI::is_item_clicked(translate_op)) m_gizmo_op = GUI::GizmoOperation::translate;
            if(GUI::is_item_clicked(rotate_op)) m_gizmo_op = GUI::GizmoOperation::rotate;
            if(GUI::is_item_clicked(scale_op)) m_gizmo_op = GUI::GizmoOperation::scale;
            if(GUI::is_item_clicked(render_mode_button) && !options.empty())
            {
                settings.mode = (SceneRendererMode)options[(current_mode_index + 1) % options.size()].value;
            }
            if(GUI::is_item_clicked(profiling_button))
            {
                settings.frame_profiling = !settings.frame_profiling;
            }

            Float2 scene_pos(viewport_pos.x, viewport_pos.y + toolbar_height);
            Float2 scene_sz(viewport_size.x, max(viewport_size.y - toolbar_height, 1.0f));
            scene_sz.x = max(scene_sz.x - 1.0f, 1.0f);
            scene_sz.y = max(scene_sz.y - 5.0f, 1.0f);

            GUI::FrameDesc gui_frame = GUI::get_frame_desc(context);
            f32 dpi_scale = max(gui_frame.dpi_scale, 1.0f);
            settings.screen_size = UInt2U((u32)(scene_sz.x * dpi_scale), (u32)(scene_sz.y * dpi_scale));

            // Draw Overlays.
            luexp(m_renderer.command_buffer->submit({}, {}, true));

            GUI::CanvasItemLayout scene_canvas;
            scene_canvas.anchor_min = Float2U(0.0f, 0.0f);
            scene_canvas.anchor_max = Float2U(1.0f, 1.0f);
            scene_canvas.offset_min = Float2U(0.0f, toolbar_height);
            scene_canvas.offset_max = Float2U(-1.0f, -5.0f);
            GUI::set_next_canvas_item_layout(context, scene_canvas);
            GUI::image(context, m_renderer.render_texture.get(), GUI::Size(), GUI::ImageFlag::flip_y);

            auto& scene_settings = s->settings;

            // Draw GUI Overlays.
            {
                // Draw gizmo.
                Actor* actor = m_world.get_actor(m_editing_actor_guid);
                if (actor && actor != camera_actor)
                {
                    Float4x4 world_mat = actor->get_local_to_world_matrix();
                    bool edited = false;
                    GUI::gizmo(context, "Scene Transform Gizmo", world_mat, camera_actor->get_world_to_local_matrix(), camera_component->get_projection_matrix(),
                        RectF(scene_pos.x, scene_pos.y, scene_sz.x, scene_sz.y), m_gizmo_op, m_gizmo_mode, 0.0f, true, false, nullptr, nullptr, nullptr, &edited);
                    if (edited)
                    {
                        actor->set_local_to_world_matrix(world_mat);
                        // Write transform back to scene data.
                        const Transform* transform = actor->get_transform();
                        s->get_actor(m_editing_actor_guid)->transform = *transform;
                    }
                }

                if (m_renderer.get_settings().frame_profiling)
                {
                    f32 debug_y = scene_pos.y + 6.0f;
                    auto draw_debug_text = [&](const c8* text)
                    {
                        GUI::draw_text(context, RectF(scene_pos.x + 8.0f, debug_y, max(scene_sz.x - 16.0f, 1.0f), 18.0f),
                            text, Color::white(), 14.0f, GUI::TextAlignment::begin, GUI::TextAlignment::center);
                        debug_y += 18.0f;
                    };
                    String debug_text;
                    strprintf(debug_text, "Frame Size: %ux%u", (u32)(scene_sz.x * dpi_scale), (u32)(scene_sz.y * dpi_scale));
                    draw_debug_text(debug_text.c_str());
                    f32 fps = gui_frame.delta_time > 0.0f ? 1.0f / gui_frame.delta_time : 0.0f;
                    strprintf(debug_text, "FPS: %f", fps);
                    draw_debug_text(debug_text.c_str());
                    for (usize i = 0; i < m_renderer.pass_time_intervals.size(); ++i)
                    {
                        f64 interval = m_renderer.pass_time_intervals[i];
                        if(interval < 0.001)
                        {
                            strprintf(debug_text, "%s: %fus", m_renderer.enabled_passes[i].c_str(), m_renderer.pass_time_intervals[i] * 1000000.0);
                        }
                        else
                        {
                            strprintf(debug_text, "%s: %fms", m_renderer.enabled_passes[i].c_str(), m_renderer.pass_time_intervals[i] * 1000.0);
                        }
                        draw_debug_text(debug_text.c_str());
                    }
                }

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

                    Float2 origin_point = { scene_pos.x + gizmo_size, scene_pos.y + scene_sz.y - gizmo_size };

                    struct GizmoLine
                    {
                        Float3U line;
                        Float4U color;

                        bool operator<(const GizmoLine& rhs) const
                        {
                            // Higher depth value gets drawn first (appear first in the draw list).
                            return line.z > rhs.line.z;
                        }
                    };

                    Vector<GizmoLine> lines;
                    lines.push_back({ x_gizmo.xyz(), Color::red() });
                    lines.push_back({ y_gizmo.xyz(), Color::green() });
                    lines.push_back({ z_gizmo.xyz(), Color::blue() });

                    // Sort by depth to ensure correct drawing order.
                    sort(lines.begin(), lines.end());
                    for (auto& line : lines)
                    {
                        // Revert y axis because GUI surface coordinates point downward.
                        GUI::draw_line(context, origin_point, origin_point + Float2(line.line.x, -line.line.y), line.color, 5.0f);
                    }
                }
            }
            GUI::end_canvas_layout(context);
            GUI::pop_id(context);

            bool scene_pointer_hovered = in_bounds(GUI::get_pointer_position(context), scene_pos, scene_pos + scene_sz);
            bool navigation_started = false;
            if (!m_navigating && GUI::is_pointer_button_down(context, GUI::PointerButton::right) && scene_pointer_hovered)
            {
                m_navigating = true;
                navigation_started = true;
            }

            if (m_navigating && !GUI::is_pointer_button_down(context, GUI::PointerButton::right))
            {
                m_navigating = false;
            }

            if (m_navigating)
            {
                Float2U mouse_delta = navigation_started ? Float2U(0.0f) : GUI::get_pointer_delta(context);
                // Rotate camera based on mouse delta.
                Transform* camera_transform = camera_actor->get_transform();
                auto rot = camera_transform->rotation;
                auto rot_mat = AffineMatrix::make_rotation(rot);

                // Key control.
                auto left = AffineMatrix::left(rot_mat);
                auto forward = AffineMatrix::forward(rot_mat);
                auto up = AffineMatrix::up(rot_mat);

                f32 camera_speed = m_camera_speed;
                if (((u8)GUI::get_key_modifiers(context) & (u8)GUI::KeyModifierFlag::shift) != 0)
                {
                    camera_speed *= 2.0f;
                }

                if (GUI::is_key_down(context, GUI::Key::w))
                {
                    camera_transform->position += forward * 0.1f * camera_speed;
                }
                if (GUI::is_key_down(context, GUI::Key::a))
                {
                    camera_transform->position += + left * 0.1f * camera_speed;
                }
                if (GUI::is_key_down(context, GUI::Key::s))
                {
                    camera_transform->position += - forward * 0.1f * camera_speed;
                }
                if (GUI::is_key_down(context, GUI::Key::d))
                {
                    camera_transform->position += - left * 0.1f * camera_speed;
                }
                if (GUI::is_key_down(context, GUI::Key::q))
                {
                    camera_transform->position += - up * 0.1f * camera_speed;
                }
                if (GUI::is_key_down(context, GUI::Key::e))
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
        }
        lucatch
        {
            GUI::draw_text(context, rect, explain(luerr), Color::white(), 16.0f,
                GUI::TextAlignment::begin, GUI::TextAlignment::begin);
        }
    }

    static bool edit_transform(GUI::IContext* context, Transform* t)
    {
        bool edited = false;

        SceneGUIRow position_row = begin_scene_gui_row(context, "Position");
        GUI::ItemHandle position_item = GUI::drag_float3(context, "Position", t->position.m, 0.01f, 0.0f, 0.0f);
        edited = edited || end_scene_gui_row(context, position_row, position_item);

        Float3& euler = get_scene_edit_buffer(g_transform_rotation_edit_buffers, (usize)t, transform_rotation_to_euler_degrees(t));
        SceneGUIRow rotation_row = begin_scene_gui_row(context, "Rotation");
        GUI::ItemHandle rotation_item = GUI::drag_float3(context, "Rotation", euler.m, 0.1f, 0.0f, 0.0f);
        bool rotation_edited = end_scene_gui_row(context, rotation_row, rotation_item);
        if (rotation_edited)
        {
            Float3 radians = euler * (PI / 180.0f);
            t->rotation = Quaternion::from_euler_angles(radians);
            edited = true;
        }
        else if(!GUI::is_item_active(rotation_item) && !GUI::is_item_focused(rotation_item))
        {
            euler = transform_rotation_to_euler_degrees(t);
        }

        SceneGUIRow scale_row = begin_scene_gui_row(context, "Scale");
        GUI::ItemHandle scale_item = GUI::drag_float3(context, "Scale", t->scale.m, 0.01f, 0.0f, 0.0f);
        edited = edited || end_scene_gui_row(context, scale_row, scale_item);

        return edited;
    }

    void SceneEditor::draw_components_grid(GUI::IContext* context, const RectF& rect)
    {
        // Draw component property grid.

        constexpr f32 title_height = 24.0f;
        constexpr f32 panel_gap = 6.0f;
        GUI::draw_text(context, RectF(rect.offset_x, rect.offset_y, rect.width, title_height), "Components Grid", Color::white(), 16.0f,
            GUI::TextAlignment::begin, GUI::TextAlignment::center);

        RectF panel_rect(rect.offset_x, rect.offset_y + title_height + panel_gap, rect.width,
            max(rect.height - title_height - panel_gap, 1.0f));

        GUI::draw_rect(context, panel_rect, Float4U(0.16f, 0.19f, 0.24f, 1.0f), 5.0f);
        GUI::draw_rect(context, RectF(panel_rect.offset_x + 1.0f, panel_rect.offset_y + 1.0f, max(panel_rect.width - 2.0f, 1.0f), max(panel_rect.height - 2.0f, 1.0f)),
            Float4U(0.04f, 0.05f, 0.06f, 1.0f), 4.0f);

        GUI::LayoutDesc panel_layout;
        panel_layout.padding = GUI::EdgeInsets::all(6.0f);
        panel_layout.gap = 0.0f;
        GUI::begin_v_layout(context, "Components Grid Panel", panel_rect, panel_layout);
        GUI::begin_scroll_view(context, "Components Grid Scroll", GUI::Size::fixed(max(panel_rect.width - 12.0f, 1.0f), max(panel_rect.height - 12.0f, 1.0f)));

        Scene* s = get_asset_or_async_load_if_not_ready<Scene>(m_scene);
        {
            SceneGUIFlowScope flow(context);

            if(!s)
            {
                draw_scene_gui_text_line(context, "Scene Loading");
            }
            else
            {
                SceneActor* actor = s->get_actor(m_editing_actor_guid);

                if (actor)
                {
                    // Draw name.
                    if(m_actor_name_editing_guid != actor->guid)
                    {
                        m_actor_name_editing_guid = actor->guid;
                        m_actor_name_editing_text = actor->name.c_str();
                    }
                    SceneGUIRow name_row = begin_scene_gui_row(context, "Actor Name");
                    GUI::set_next_item_layout(context, GUI::LayoutStyle::fixed_width(72.0f));
                    GUI::text(context, "Name");
                    GUI::set_next_item_layout(context, GUI::LayoutStyle::fill_width());
                    GUI::ItemHandle name_item = GUI::input_text(context, "Actor Name", m_actor_name_editing_text);
                    if(end_scene_gui_row(context, name_row, name_item))
                    {
                        actor->name = m_actor_name_editing_text;
                        on_edit_actor_info(*actor);
                    }
                    // Draw transform.
                    if(edit_transform(context, &actor->transform))
                    {
                        on_edit_actor_transform(*actor);
                    }

                    auto& components = actor->components;

                    if (components.empty())
                    {
                        draw_scene_gui_text_line(context, "No components");
                    }
                    else
                    {
                        auto iter = components.begin();

                        while (iter != components.end())
                        {
                            auto& obj = *iter;
                            Name type_name = get_type_name(obj.type());
                            GUI::push_id(context, (const void*)obj.type().handle);
                            SceneGUIRow header_row = begin_scene_gui_row(context, type_name.c_str());
                            GUI::ItemHandle header = GUI::collapsing_header(context, type_name.c_str());
                            bool open = GUI::get_item_state(header, GUI::State::open());
                            end_scene_gui_row(context, header_row, header);
                            bool remove_component = false;
                            if (open)
                            {
                                bool edited = edit_scene_object(context, &m_world, obj.type(), obj.get());
                                if(edited)
                                {
                                    on_actor_edit_component(*actor, obj.type());
                                }

                                SceneGUIRow remove_row = begin_scene_gui_row(context, "Remove Component", 28.0f);
                                GUI::set_next_item_layout(context, GUI::LayoutStyle::fixed_width(120.0f));
                                GUI::ItemHandle remove_button = GUI::button(context, "Remove");
                                remove_component = end_scene_gui_row(context, remove_row, remove_button);
                            }
                            GUI::pop_id(context);

                            if (remove_component)
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

                    SceneGUIRow new_component_row = begin_scene_gui_row(context, "New Component", 30.0f);
                    GUI::set_next_item_layout(context, GUI::LayoutStyle::fixed_width(150.0f));
                    GUI::ItemHandle new_component_button = GUI::button(context, "New Component");
                    end_scene_gui_row(context, new_component_row, new_component_button);
                    if(GUI::is_item_clicked(new_component_button))
                    {
                        m_new_component_popup_open = !m_new_component_popup_open;
                        m_new_component_popup_position = GUI::get_pointer_position(context);
                        if(m_new_component_popup_open)
                        {
                            GUI::open_popup(context, m_new_component_popup_handle);
                        }
                        else
                        {
                            GUI::close_popup(context, m_new_component_popup_handle);
                        }
                    }
                    {
                        f32 popup_width = 240.0f;
                        f32 popup_height = max((f32)g_env->component_types.size() * 26.0f + 10.0f, 36.0f);
                        Vector<Pair<typeinfo_t, GUI::ItemHandle>> component_items;
                        bool popup_open = GUI::begin_popup(context, "New Component Popup", m_new_component_popup_position, GUI::Size::fixed(popup_width, popup_height), &m_new_component_popup_handle);
                        if(popup_open)
                        {
                            m_new_component_popup_open = true;
                            component_items.reserve(g_env->component_types.size());
                            for (auto& i : g_env->component_types)
                            {
                                bool exists = false;
                                for(auto& c : components)
                                {
                                    if(c.type() == i)
                                    {
                                        exists = true;
                                        break;
                                    }
                                }
                                auto comp_name = get_type_name(i);
                                if (!exists)
                                {
                                    component_items.push_back(make_pair(i, GUI::selectable(context, comp_name.c_str())));
                                }
                                else
                                {
                                    GUI::text(context, comp_name.c_str());
                                }
                            }
                            GUI::end_popup(context);
                        }
                        else if(m_new_component_popup_open && !GUI::is_popup_open(context, m_new_component_popup_handle))
                        {
                            m_new_component_popup_open = false;
                        }
                        for(auto& item : component_items)
                        {
                            if(GUI::is_item_clicked(item.second))
                            {
                                object_t comp = object_alloc(item.first);
                                construct_type(item.first, comp);
                                ObjRef comp_obj;
                                comp_obj.attach(comp);
                                components.push_back(move(comp_obj));
                                on_actor_add_component(*actor, item.first);
                                m_new_component_popup_open = false;
                                GUI::close_popup(context, m_new_component_popup_handle);
                            }
                        }
                    }
                }
                else
                {
                    draw_scene_gui_text_line(context, "Select an entity to see components.");
                }
            }
        }
        GUI::end_scroll_view(context);
        GUI::end_v_layout(context);
    }
    void SceneEditor::on_render(GUI::IContext* context)
    {
        if(!m_open)
        {
            return;
        }

        GUI::push_id(context, this);
        GUI::DockPanelStyle panel_style;
        panel_style.floating_size = Float2U(1000.0f, 500.0f);
        panel_style.min_floating_size = Float2U(420.0f, 260.0f);
        GUI::LayoutDesc panel_layout;
        panel_layout.padding = GUI::EdgeInsets::all(0.0f);
        panel_layout.gap = 0.0f;
        GUI::ItemHandle panel = GUI::begin_dock_panel(context, "Scene Editor", &m_open, panel_style, panel_layout);
        RectF panel_rect = GUI::get_item_state(panel, GUI::State::rect());
        if(panel_rect.width <= 1.0f || panel_rect.height <= 1.0f)
        {
            GUI::text(context, "Scene Editor");
            GUI::end_dock_panel(context);
            GUI::pop_id(context);
            return;
        }
        GUI::push_clip_rect(context, panel_rect);

        auto s = get_asset_or_async_load_if_not_ready<Scene>(m_scene);
        if (!s)
        {
            draw_scene_gui_text_line(context, "Asset Unloaded");
            GUI::pop_clip_rect(context);
            GUI::end_dock_panel(context);
            GUI::pop_id(context);
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
            draw_scene_gui_text_line(context, "Scene Loading");
            GUI::pop_clip_rect(context);
            GUI::end_dock_panel(context);
            GUI::pop_id(context);
            return;
        }

        bool capture_scene = false;
        Path capture_save_path;

        constexpr f32 menu_height = 30.0f;
        constexpr f32 content_gap = 6.0f;
        GUI::begin_menu_bar(context, "Scene Editor Menu Bar", RectF(panel_rect.offset_x, panel_rect.offset_y, min(panel_rect.width, 138.0f), menu_height));
        GUI::ItemHandle save_item;
        if(GUI::begin_menu(context, "File"))
        {
            save_item = GUI::menu_item(context, "Save");
            GUI::end_menu(context);
        }
        GUI::ItemHandle capture_item;
        if(GUI::begin_menu(context, "Tools"))
        {
            capture_item = GUI::menu_item(context, "Capture scene");
            GUI::end_menu(context);
        }
        GUI::end_menu_bar(context);

        if(GUI::is_item_clicked(save_item))
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
        if(GUI::is_item_clicked(capture_item))
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

        Float2 content_pos(panel_rect.offset_x, panel_rect.offset_y + menu_height + content_gap);
        Float2 content_size(panel_rect.width, max(panel_rect.height - menu_height - content_gap, 1.0f));
        content_size.x = max(content_size.x, 1.0f);
        content_size.y = max(content_size.y, 1.0f);

        constexpr f32 column_gap = 8.0f;
        f32 left_width = min(max(content_size.x * 0.24f, 240.0f), 360.0f);
        f32 right_width = min(max(content_size.x * 0.28f, 280.0f), 420.0f);
        f32 center_width = content_size.x - left_width - right_width - column_gap * 2.0f;
        if(center_width < 260.0f)
        {
            f32 side_budget = max(content_size.x - 260.0f - column_gap * 2.0f, 2.0f);
            left_width = max(side_budget * 0.45f, 1.0f);
            right_width = max(side_budget - left_width, 1.0f);
            center_width = max(content_size.x - left_width - right_width - column_gap * 2.0f, 1.0f);
        }

        RectF left_rect(content_pos.x, content_pos.y, left_width, content_size.y);
        RectF center_rect(left_rect.offset_x + left_rect.width + column_gap, content_pos.y, center_width, content_size.y);
        RectF right_rect(center_rect.offset_x + center_rect.width + column_gap, content_pos.y, right_width, content_size.y);

        f32 actor_list_height = max((left_rect.height - column_gap) * 0.48f, min(180.0f, left_rect.height));
        actor_list_height = min(actor_list_height, max(left_rect.height - column_gap, 1.0f));
        RectF actor_list_rect(left_rect.offset_x, left_rect.offset_y, left_rect.width, actor_list_height);
        RectF scene_settings_rect(left_rect.offset_x, left_rect.offset_y + actor_list_height + column_gap, left_rect.width,
            max(left_rect.height - actor_list_height - column_gap, 1.0f));

        draw_actor_list(context, actor_list_rect);
        draw_scene_settings(context, scene_settings_rect);
        draw_scene(context, center_rect);
        draw_components_grid(context, right_rect);

        GUI::pop_clip_rect(context);
        GUI::end_dock_panel(context);
        GUI::pop_id(context);

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
