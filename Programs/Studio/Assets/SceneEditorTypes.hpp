/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#pragma once
#include "Scene.hpp"
#include "../SceneRenderer.hpp"
#include "../MainEditor.hpp"
#include "../Scene.hpp"
#include "../SceneSettings.hpp"
#include "../Camera.hpp"
#include "../World.hpp"
#include <Luna/Runtime/Math/Color.hpp>
#include <Luna/RHIUtility/ResourceReadContext.hpp>
#include <Luna/RHIUtility/ResourceWriteContext.hpp>
#include <Luna/Image/RHIHelper.hpp>
#include <Luna/Runtime/Random.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include <Luna/GUI/Legacy/Editor.hpp>
#include "SceneEditorTypes.generated.hpp"

namespace Luna
{
    struct [[luna::struct("{5b4aea33-e61a-4042-ba91-1f4ec84f8194}")]] SceneEditorUserData
    {
        // Resources for rendering grids.
        Ref<RHI::IBuffer> m_grid_vb;
        Ref<RHI::IDescriptorSetLayout> m_grid_dlayout;
        Ref<RHI::IPipelineLayout> m_grid_playout;
        Ref<RHI::IPipelineState> m_grid_pso;

        SceneEditorUserData() {}

        RV init();
    };

    struct [[luna::struct("{c973cc28-78e7-4be5-a391-8c2e5960fa48}")]] SceneEditor : public IAssetEditor
    {
    public:
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
        bool m_new_component_popup_open = false;
        Float2U m_new_component_popup_position = Float2U(0.0f);

        // States for scene viewport.

        GUI::GizmoMode m_gizmo_mode = GUI::GizmoMode::local;
        GUI::GizmoOperation m_gizmo_op = GUI::GizmoOperation::translate;

        f32 m_camera_speed = 1.0f;

        bool m_navigating = false;
        bool m_scene_pointer_initialized = false;
        Float2U m_last_scene_pointer = Float2U(0.0f);

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

        void draw_actor_list(GUICore::IContext* context, const GUICore::LayoutConfig& layout);
        void draw_actor_tree_node(GUICore::IContext* context, Actor* actor, bool& open_actor_list_popup);
        void draw_scene_settings(GUICore::IContext* context, const GUICore::LayoutConfig& layout);
        void draw_scene(GUICore::IContext* context, const GUICore::LayoutConfig& layout);
        void draw_components_grid(GUICore::IContext* context, const GUICore::LayoutConfig& layout);
        virtual void on_render(GUICore::IContext* context, const GUICore::LayoutConfig& layout) override;
        virtual bool closed() override
        {
            return !m_open;
        }
        void capture_scene_to_file(const Path& path);
    };
}
