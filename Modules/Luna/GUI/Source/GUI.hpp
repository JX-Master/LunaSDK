/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUI.hpp
* @author JXMaster
* @date 2026/5/21
*/
#pragma once
#include "../GUI.hpp"
#include <Luna/Runtime/HashSet.hpp>
#include <Luna/Runtime/TSAssert.hpp>
#include <Luna/VG/ShapeDrawList.hpp>
#include <Luna/VG/ShapeRenderer.hpp>
#include <Luna/VG/TextArranger.hpp>
#include <Luna/VG/Shapes.hpp>
#include <Luna/Font/Font.hpp>

namespace Luna
{
    namespace GUI
    {
        struct GUIIDHash
        {
            usize operator()(GUIID value) const
            {
                return (usize)value;
            }
        };

        struct ItemResult
        {
            u64 generation = 0;
            HashMap<Name, Any> states;
        };

        struct PersistentItemState
        {
            bool open = true;
            bool active = false;
            bool focused = false;
            bool pointer_down = false;
            f32 scroll_y = 0.0f;
            f64 last_click_time = -1000.0;
        };

        struct NodeLayout
        {
            RectF rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            RectF clip_rect = RectF(0.0f, 0.0f, 0.0f, 0.0f);
            GUILayoutMetrics metrics;
            bool metrics_valid = false;
        };

        struct GUIContext : IGUIContext
        {
            lustruct("GUI::GUIContext", "{BF721C36-C7C2-4B49-89E6-22F0B3BE56F5}");
            luiimpl();
            lutsassert_lock();

            Ref<RHI::IDevice> m_device;
            GUIFrameDesc m_frame_desc;
            GUIDescription m_build_desc;
            GUIDescription m_submitted_desc;
            Vector<NodeLayout> m_layouts;
            Vector<GUIInputEvent> m_input_events;
            Vector<u32> m_parent_stack;
            Vector<GUIID> m_id_stack;
            Vector<u32> m_child_ordinals;
            HashMap<GUIID, ItemResult, GUIIDHash> m_last_results;
            HashMap<GUIID, ItemResult, GUIIDHash> m_current_results;
            HashMap<GUIID, PersistentItemState, GUIIDHash> m_persistent_states;
            GUIID m_active_id = 0;
            GUIID m_focused_id = 0;
            GUIID m_hovered_id = 0;
            Float2U m_pointer_pos = Float2U(0.0f);
            bool m_pointer_inside = false;
            bool m_submitted = false;
            bool m_has_next_item_layout = false;
            GUILayoutStyle m_next_item_layout;
            u64 m_generation = 0;
            f64 m_time = 0.0;
            Ref<VG::IShapeDrawList> m_shape_draw_list;
            Ref<VG::IShapeRenderer> m_shape_renderer;
            Ref<VG::IFontAtlas> m_font_atlas;

            GUIContext();

            virtual void begin_frame(const GUIFrameDesc& desc) override;
            virtual void add_input_event(const GUIInputEvent& event) override;
            virtual void add_input_events(Span<const GUIInputEvent> events) override;
            virtual R<GUIDescription> end_build() override;
            virtual RV submit(const GUIDescription& desc) override;
            virtual RV render(RHI::ICommandBuffer* cmdbuf, RHI::ITexture* render_target) override;

            GUIItemHandle add_node(GUINodeKind kind, const c8* text, bool interactive);
            void begin_container(GUINodeKind kind, const c8* label, const GUISize& size, GUIItemHandle* out_handle);
            void end_container();
            const Any* get_state(GUIItemHandle handle, const Name& key);
            void set_state(GUIItemHandle handle, const Name& key, const Any& value);
            void remove_state(GUIItemHandle handle, const Name& key);
            void set_next_item_layout(const GUILayoutStyle& style);
            void push_id(GUIID id);
            void pop_id();

            ItemResult* get_query_result(GUIItemHandle handle);
            ItemResult& get_or_create_current_result(GUIID id);
            PersistentItemState& get_or_create_persistent_state(GUIID id);
            RectF layout_node(u32 node_index, const RectF& rect, const RectF& clip_rect);
            GUILayoutMetrics measure_node(u32 node_index);
            GUIID hit_test(const Float2U& pos) const;
            GUIID hit_test_node_kind(const Float2U& pos, GUINodeKind kind) const;
            GUINode* find_node(GUIID id);
            void update_float_node_from_pointer(GUIID id, const Float2U& pos);
            void process_input_events();
            void render_node(u32 node_index);
            void render_rect(const RectF& rect, const RectF& clip_rect, const Float4U& color, f32 radius, RHI::ITexture* texture = nullptr);
            void render_text(const RectF& rect, const RectF& clip_rect, const c8* text, f32 font_size, const Float4U& color, VG::TextAlignment horizontal_alignment, VG::TextAlignment vertical_alignment = VG::TextAlignment::center);
            RectF to_vg_rect(const RectF& rect) const;
        };
    }
}
