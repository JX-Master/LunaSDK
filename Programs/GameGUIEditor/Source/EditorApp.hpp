/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorApp.hpp
* @author JXMaster
* @date 2026/8/28
*/
#pragma once
#include "DocumentFileSystem.hpp"
#include "../Service/GameGUIEditorService.hpp"
#include <Luna/Asset/Asset.hpp>
#include <Luna/EditorGUI/EditorGUI.hpp>
#include <Luna/GameGUI/GameGUI.hpp>
#include <Luna/GUIWindow/GUIWindow.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/RHI/SwapChain.hpp>
#include <Luna/RHIUtility/BlitContext.hpp>
#include <Luna/Runtime/Guid.hpp>
#include <Luna/Runtime/Path.hpp>
#include <Luna/Window/Window.hpp>
#if defined(LUNA_PLATFORM_MACOS)
#include <Luna/Window/ApplicationMenu.hpp>
#endif

namespace Luna
{
    namespace GameGUIEditor
    {
        namespace Internal
        {
            constexpr c8 APP_NAME[] = "Luna GameGUI Editor";
            constexpr u32 PREVIEW_LAYOUT_WIDTH = 640;
            constexpr u32 PREVIEW_LAYOUT_HEIGHT = 480;
            constexpr f32 PREVIEW_GRID_SIZE = 32.0f;
            constexpr f32 PREVIEW_RESIZE_HANDLE_SIZE = 14.0f;
            constexpr f32 PREVIEW_MIN_NODE_SIZE = 64.0f;
            constexpr f32 PREVIEW_MAX_NODE_SIZE = 8192.0f;
            constexpr f32 PREVIEW_ZOOM_LEVELS[] = {
                0.125f, 0.25f, 0.5f, 0.75f, 1.0f, 1.25f,
                1.5f, 2.0f, 3.0f, 4.0f, 6.0f, 8.0f
            };
            constexpr usize PREVIEW_ZOOM_LEVEL_COUNT =
                sizeof(PREVIEW_ZOOM_LEVELS) / sizeof(PREVIEW_ZOOM_LEVELS[0]);

            enum class PropertyTarget : u8
            {
                node,
                attachment
            };

            enum class VisualEffectAction : u8
            {
                add,
                move_up,
                move_down,
                remove
            };

            struct VisualEffectEditor
            {
                Variant source;
                bool supported = true;
                i32 phase = 0;
                i32 type = 0;
                f32 inset[4] = {0.0f, 0.0f, 0.0f, 0.0f};
                f32 color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
                f32 gradient_colors[3][4] = {
                    {1.0f, 1.0f, 1.0f, 1.0f},
                    {1.0f, 1.0f, 1.0f, 1.0f},
                    {1.0f, 1.0f, 1.0f, 1.0f}
                };
                f32 radius = 0.0f;
                f32 line_width = 1.0f;
                f32 shadow_offset[2] = {0.0f, 0.0f};
                f32 shadow_softness = 0.0f;
                f32 shadow_spread = 0.0f;
                i32 shadow_mode = 0;
            };

            struct PropertyEditor
            {
                EditingPropertyDesc desc;
                PropertyTarget target = PropertyTarget::node;
                Variant original;
                Variant alternate_original;
                Variant baseline;
                bool original_present = false;
                bool alternate_present = false;
                bool raw = false;
                String text;
                bool boolean = false;
                f32 number = 0.0f;
                f32 vector[4] = {0.0f, 0.0f, 0.0f, 0.0f};
                i32 selected_item = 0;
                i32 size_mode = 0;
                i32 baseline_size_mode = 0;
                Vector<VisualEffectEditor> visual_effects;
            };

            struct PreviewState
            {
                Ref<GUI::IContext> context;
                Ref<GameGUI::IInstance> instance;
                Ref<RHI::ITexture> target;
                u64 revision = 0;
                Float2U viewport_size = Float2U((f32)PREVIEW_LAYOUT_WIDTH,
                    (f32)PREVIEW_LAYOUT_HEIGHT);
                UInt2U render_size = UInt2U(PREVIEW_LAYOUT_WIDTH, PREVIEW_LAYOUT_HEIGHT);
                Float2U pending_viewport_size = Float2U((f32)PREVIEW_LAYOUT_WIDTH,
                    (f32)PREVIEW_LAYOUT_HEIGHT);
                UInt2U pending_render_size = UInt2U(PREVIEW_LAYOUT_WIDTH,
                    PREVIEW_LAYOUT_HEIGHT);
                Float2U pan = Float2U(0.0f);
                Float2U pan_pointer = Float2U(0.0f);
                f32 zoom = 1.0f;
                f32 zoom_wheel_accumulator = 0.0f;
                Float2U node_size = Float2U((f32)PREVIEW_LAYOUT_WIDTH,
                    (f32)PREVIEW_LAYOUT_HEIGHT);
                Float2U resize_pointer = Float2U(0.0f);
                Float2U resize_start_size = Float2U(0.0f);
                GUI::id_t selected_element_id = 0;
                bool panning = false;
                bool resizing = false;
            };

            enum class HierarchyDropMode : u8
            {
                none,
                before,
                after,
                child
            };

            struct HierarchyDragState
            {
                Guid source;
                Float2U press_position = Float2U(0.0f);
                bool pressed = false;
                bool dragging = false;
                HierarchyDropMode drop_mode = HierarchyDropMode::none;
                Guid target_node;
                Guid target_parent;
                usize target_index = 0;
                RectF feedback_rect;
                u32 feedback_depth = 0;
            };

            struct DocumentView
            {
                u64 id = 0;
                u64 revision = 0;
                u64 history_state = 0;
                String title;
                String asset_path;
                Guid asset_guid;
                bool dirty = false;
                bool can_undo = false;
                bool can_redo = false;
                bool panel_open = true;
                Ref<AuthoringDocument> snapshot;
                Ref<GameGUI::Document> cooked_snapshot;
                Variant diagnostics;
                Guid selected_node;
                u64 inspector_revision = 0;
                Guid inspector_node;
                String node_name;
                Vector<PropertyEditor> property_editors;
                PreviewState preview;
                HierarchyDragState hierarchy_drag;
                Guid hierarchy_context_node;
                Float2U hierarchy_context_position = Float2U(0.0f);
            };

            struct NodeTypeView
            {
                Guid type;
                String name;
                String display_name;
                String category;
                EditingSchema property_schema;
                EditingSchema child_attachment_schema;
            };

            struct NodeHit
            {
                Guid node;
                Guid parent;
                usize sibling_index = 0;
                u32 depth = 0;
                GUI::ElementHandle element;
            };

            struct TypeHit
            {
                Guid type;
                GUI::ElementHandle element;
            };

            struct PropertyActionHit
            {
                usize property_index = 0;
                GUI::ElementHandle element;
            };

            struct VisualEffectActionHit
            {
                usize property_index = 0;
                usize effect_index = 0;
                VisualEffectAction action = VisualEffectAction::add;
                GUI::ElementHandle element;
            };

            struct UIHandles
            {
                u64 document_id = 0;
                GUI::ElementHandle create_document;
                GUI::ElementHandle open_document;
                GUI::ElementHandle save_document;
                GUI::ElementHandle save_as_document;
                GUI::ElementHandle cook_document;
                GUI::ElementHandle close_document;
                GUI::ElementHandle undo;
                GUI::ElementHandle redo;
                GUI::ElementHandle set_root_node;
                GUI::ElementHandle delete_node;
                GUI::ElementHandle preview_host;
                Vector<NodeHit> nodes;
                Vector<TypeHit> types;
                Vector<PropertyActionHit> browse_assets;
                Vector<VisualEffectActionHit> visual_effect_actions;
            };

            struct PreviewInput
            {
                u64 document_id = 0;
                Vector<GUI::InputEvent> events;
            };

            struct EditorApp
            {
                Ref<Window::IWindow> window;
                Ref<RHI::ISwapChain> swap_chain;
                Ref<RHI::ICommandBuffer> cmdbuf;
                Ref<RHI::ITexture> gui_target;
                Ref<RHIUtility::IBlitContext> blit;
                Ref<GUI::IContext> gui;
                Ref<GUI::IRenderer> renderer;
                Ref<GUI::IRenderer> preview_renderer;
                UniquePtr<GameGUIEditor::Service> service;
                GUIWindow::GUIWindowInputAdapter input_adapter;
                Vector<DocumentView> documents;
                Vector<u64> deferred_document_removals;
                Vector<NodeTypeView> node_types;
                i32 selected_document = 0;
                bool dock_layout_initialized = false;
                bool discard_smoke = false;
                String workspace_path;
                Path workspace_root;
                DocumentFileSystem document_files;
                String error_message;
                u32 queue = U32_MAX;
                u32 width = 0;
                u32 height = 0;
                i32 max_frames = -1;
#if defined(LUNA_PLATFORM_MACOS)
                bool application_menu_has_document = false;
                bool application_menu_can_cook = false;
                bool application_menu_can_undo = false;
                bool application_menu_can_redo = false;
#endif

                RV init();
                RV run();
                RV resize_target(const UInt2U& size);
                RV initialize_dock_layout();
                bool invoke(const c8* url, const Variant& params, Variant& result);
                bool native_path_to_asset_path(Path native_path, String& asset_path,
                    bool allow_missing = false);
                void show_file_error(const c8* title);
                bool create_document();
                bool open_document();
                bool refresh_snapshot(DocumentView& document);
                bool update_metadata(DocumentView& document, const Variant& metadata);
                DocumentView* find_document(u64 id);
                DocumentView* active_document();
                void place_document_panel(u64 document_id, u64 target_document_id);
                void rebuild_inspector(DocumentView& document);
                RV apply_preview_surface_size(DocumentView& document);
                RV ensure_preview(DocumentView& document);
                RV build_preview(DocumentView& document, Span<const GUI::InputEvent> input);
                RV render_frame(DocumentView* preview_document);
                void collect_preview_input(DocumentView& document,
                    const GUI::ElementHandle& preview_host, const RectF& preview_rect,
                    const GUI::FrameDesc& editor_frame, PreviewInput& preview_input);
                GUI::ElementHandle build_editor(UIHandles& handles);
#if !defined(LUNA_PLATFORM_MACOS)
                void build_main_menu_bar(UIHandles& handles);
#endif
                void build_hierarchy_panel(UIHandles& handles);
                void build_palette_panel(UIHandles& handles);
                void build_document_panels(UIHandles& handles);
                void build_inspector_panel(UIHandles& handles);
                void build_diagnostics_panel();
                GUI::ElementHandle build_visual_effects_editor(PropertyEditor& property,
                    usize property_index, GUI::id_t property_id, UIHandles& handles);
                bool process_visual_effect_actions(DocumentView& document,
                    const UIHandles& handles);
                void build_hierarchy_node(DocumentView& document, const Guid& node,
                    const Guid& parent, usize sibling_index, u32 depth, UIHandles& handles);
                void process_interactions(const UIHandles& handles);
                bool process_hierarchy_interactions(DocumentView& document,
                    const UIHandles& handles);
                void update_hierarchy_drop(DocumentView& document, const UIHandles& handles,
                    const Float2U& pointer_position);
                bool apply_hierarchy_drop(DocumentView& document);
                void apply_inspector_changes(DocumentView& document);
                void remove_document_view(u64 id);
                bool has_dirty_documents() const;
                bool confirm_exit();
                void request_close(DocumentView& document, bool discard);
                void save(DocumentView& document, bool save_as);
                void cook(DocumentView& document);
                void undo_document(DocumentView& document);
                void redo_document(DocumentView& document);
#if defined(LUNA_PLATFORM_MACOS)
                RV install_application_menu();
                RV update_application_menu_state();
                void handle_application_menu_item(Window::application_menu_item_id_t id);
#endif
            };

            GUI::LayoutConfig fill_layout();
            GUI::LayoutConfig fill_width(f32 height);
            GUI::LayoutConfig fixed_layout(f32 width, f32 height);
            String guid_string(const Guid& guid);
            bool decode_guid_string(const Variant& value, Guid& result);
            GUI::id_t guid_gui_id(GUI::id_t scope, const Guid& guid);
            GUI::id_t document_panel_id(GUI::IContext* context, u64 document_id);
            GUI::id_t hierarchy_context_popup_id(GUI::IContext* context,
                u64 document_id);
            String property_text(const Variant& value);
            bool decode_editing_schema(const Variant& value, EditingSchema& schema);
            R<Variant> property_value(const PropertyEditor& editor);
            void decode_visual_effects(const Variant& value,
                Vector<VisualEffectEditor>& effects);
            Variant encode_visual_effects(Span<const VisualEffectEditor> effects);
            f32 visual_effects_editor_height(const PropertyEditor& property);
            bool point_in_rect(const RectF& rect, const Float2U& point);
            RectF item_screen_rect(GUI::IContext* context,
                const GUI::ElementHandle& item, bool clip_rect = false);
            bool subtree_contains(const AuthoringDocument& document, const Guid& root,
                const Guid& node);
            bool find_parent_info(const AuthoringDocument& document, const Guid& node,
                Guid& parent, usize& sibling_index);
            Variant editing_params(const DocumentView& document);
            R<GUI::paint_order_id_t> draw_hierarchy_drop(GUI::IContext* context,
                const GUI::ElementHandle& element, GUI::DrawPhase phase,
                GUI::paint_order_id_t paint_order_id, void* userdata);
        }
    }
}
