/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GUICore.hpp
* @author JXMaster
* @date 2026/6/17
*/
#pragma once
#include "../GUICore.hpp"
#include <Luna/Runtime/TSAssert.hpp>
#include <Luna/VG/FontAtlas.hpp>
#include "GUICore.generated.hpp"

namespace Luna
{
    namespace GUICore
    {
        struct StateRecord
        {
            ObjRef data;
            StateLifetime lifetime = StateLifetime::next_frame;
            u32 last_touched_generation = 0;
        };

        struct DragDropPayloadStorage
        {
            Name type;
            Vector<byte_t> data;
            ElementHandle source;
            ElementHandle target;
            bool delivery = false;
        };

        struct DragDropState
        {
            bool active = false;
            id_t source_id = 0;
            Name type;
            Vector<byte_t> data;
            HashMap<id_t, DragDropPayloadStorage, IdHash> deliveries;
            DragDropPayload payload_view;
        };

        struct FontResource
        {
            Ref<Font::IFontFile> font;
            u32 font_index = 0;
        };

        struct TextInputRequest
        {
            id_t element_id = 0;
            i32 cursor = 0;
        };

        struct [[Luna::struct("{FA7AB346-9939-47ED-8506-6B4D5F9C5F4C}")]] Context : IContext
        {
            luiimpl();
            lutsassert_lock();

            FrameDesc m_frame_desc;
            Vector<Layer> m_layers;
            Vector<Element> m_elements;
            Vector<DrawCommand> m_draw_commands;
            Vector<InputEvent> m_input_events;
            Vector<DebugIssueInfo> m_debug_issues;
            Vector<DebugPassInfo> m_debug_passes;
            Vector<u32> m_layer_stack;
            Vector<u32> m_element_stack;
            Vector<Name> m_style_stack;
            Vector<id_t> m_data_scope_stack;
            HashMap<id_t, u32, IdHash> m_element_indices;
            HashMap<id_t, StateRecord, IdHash> m_states;
            HashMap<Name, Style> m_styles;
            Vector<StyleEntrySchema> m_style_schemas;
            HashMap<Name, FontResource> m_fonts;
            ClipboardIO m_clipboard_io;
            TextInputRequest m_text_input_request;
            HashMap<id_t, InteractionState, IdHash> m_interactions;
            HashMap<id_t, Vector<InputEvent>, IdHash> m_input_deliveries;
            HashMap<id_t, Vector<RoutedInputEvent>, IdHash> m_routed_input_deliveries;
            DragDropState m_drag_drop;
            Ref<VG::IFontAtlas> m_font_atlas;
            id_t m_hovered_element = 0;
            id_t m_active_element = 0;
            id_t m_focused_element = 0;
            id_t m_last_clicked_element = 0;
            Float2U m_pointer_position = Float2U(0.0f);
            Float2U m_pointer_delta = Float2U(0.0f);
            Float2U m_last_clicked_position = Float2U(0.0f);
            f32 m_elapsed_time = 0.0f;
            f32 m_last_clicked_time = -1000.0f;
            bool m_pointer_inside = false;
            bool m_pointer_down[5] = {};
            bool m_key_down[256] = {};
            KeyModifierFlag m_key_modifiers = KeyModifierFlag::none;
            u32 m_generation = 0;
            PerformanceCounters m_counters;

            virtual void begin_frame(const FrameDesc& desc) override;
            virtual u32 generation() const override;
            virtual FrameDesc get_frame_desc() const override;
            virtual Float2U get_pointer_position() const override;
            virtual Float2U get_pointer_delta() const override;
            virtual bool is_pointer_inside() const override;
            virtual bool is_pointer_button_down(PointerButton button) const override;
            virtual bool is_key_down(KeyCode key) const override;
            virtual KeyModifierFlag get_key_modifiers() const override;
            virtual void add_input_event(const InputEvent& event) override;
            virtual void add_input_events(Span<const InputEvent> events) override;
            virtual void push_layer(id_t id, const Float2U& screen_position = Float2U(0.0f), const Name& debug_name = Name()) override;
            virtual void pop_layer() override;
            virtual void push_data_scope(id_t id) override;
            virtual void pop_data_scope() override;
            virtual id_t current_data_scope() const override;
            virtual id_t make_id(id_t local_id) const override;
            virtual id_t make_id(const c8* local_name) const override;
            virtual ElementHandle begin_element(id_t id, const Name& debug_name = Name()) override;
            virtual void end_element() override;
            virtual void set_layout(const ElementHandle& element, const LayoutInput& layout) override;
            virtual void set_layout_config(const ElementHandle& element, const LayoutConfig& config) override;
            virtual void set_layout_result(const ElementHandle& element, const LayoutResult& result) override;
            virtual RV apply_layout(const ElementHandle& root, const RectF& rect) override;
            virtual void set_interactable(const ElementHandle& element, const Interactable& interactable) override;
            virtual void set_navigation_config(const ElementHandle& element, const NavigationConfig& navigation) override;
            virtual NavigationConfig get_navigation_config(const ElementHandle& element) const override;
            virtual void set_hit_test_config(const ElementHandle& element, const ElementHitTestConfig& hit_test) override;
            virtual ElementHitTestConfig get_hit_test_config(const ElementHandle& element) const override;
            virtual void set_drag_drop_source_types(const ElementHandle& element, Span<const Name> types) override;
            virtual void set_drag_drop_target_types(const ElementHandle& element, Span<const Name> types) override;
            virtual void bind_style(const ElementHandle& element, const Name& style) override;
            virtual const Element* get_element(u32 index) const override;
            virtual const Element* find_element(id_t id) const override;
            virtual ElementHandle find_element_handle(id_t id) const override;
            virtual void draw(const DrawCommand& command) override;
            virtual void draw_for_element(const ElementHandle& element, const DrawCommand& command) override;
            virtual RV compile_draw_commands(VG::IShapeDrawList* draw_list) override;
            virtual RV register_font(const Name& id, Font::IFontFile* font, u32 font_index = 0) override;
            virtual FontDesc get_font(const Name& id) override;
            virtual void set_clipboard_io(const ClipboardIO& io) override;
            virtual ClipboardIO get_clipboard_io() override;
            virtual void request_text_input(const ElementHandle& element, i32 cursor) override;
            virtual TextInputState get_text_input_state() override;
            virtual void route_input() override;
            virtual ElementHandle hit_test(const Float2U& screen_position, HitTestCallback* callback = nullptr,
                void* userdata = nullptr) const override;
            virtual InteractionState get_interaction_state(id_t id) const override;
            virtual Span<const InputEvent> get_delivered_input_events(id_t id) override;
            virtual Span<const RoutedInputEvent> get_routed_input_events(id_t id) override;
            virtual void focus_element(id_t id) override;
            virtual id_t focused_element() const override;
            virtual bool navigate_default(const NavigationRequest& request) override;
            virtual void capture_pointer(id_t id) override;
            virtual void release_pointer_capture(id_t id = 0) override;
            virtual id_t captured_element() const override;
            virtual RV start_drag_drop(const ElementHandle& source, const Name& payload_type, const void* data, usize data_size) override;
            virtual void clear_drag_drop() override;
            virtual bool is_drag_drop_active() const override;
            virtual const DragDropPayload* get_drag_drop_payload() override;
            virtual ElementHandle hit_test_drag_drop_target(const Name& payload_type, const Float2U& screen_position) const override;
            virtual const DragDropPayload* get_drag_drop_delivery(const ElementHandle& target, const Name& payload_type) override;
            virtual object_t get_state(id_t id) override;
            virtual RV set_state(id_t id, object_t data, StateLifetime lifetime = StateLifetime::next_frame) override;
            virtual void clear_state(id_t id) override;
            virtual void define_style(const Name& name, const Name& parent = Name()) override;
            virtual void set_style_parent(const Name& name, const Name& parent) override;
            virtual void set_style_value(const Name& style, const Name& entry, const StyleValue& value) override;
            virtual void inherit_style_entry(const Name& style, const Name& entry) override;
            virtual void unset_style_entry(const Name& style, const Name& entry) override;
            virtual void push_style(const Name& style) override;
            virtual void pop_style() override;
            virtual Name current_style() const override;
            virtual StyleValue get_style_value(const Name& style, const Name& entry, const StyleValue& default_value) override;
            virtual void register_style_entry_schema(const StyleEntrySchema& schema) override;
            virtual Span<const StyleEntrySchema> get_style_entry_schemas() override;
            virtual PerformanceCounters get_performance_counters() override;
            virtual DebugInfo dump_debug_info() override;
            virtual void log_debug_issue(DebugIssueSeverity severity, const Name& category, const c8* message,
                id_t element = 0) override;
            virtual void log_debug_pass(DebugPassKind kind, const Name& name, const Name& reason, id_t element = 0,
                const c8* detail = nullptr, f64 duration_ms = 0.0) override;

            void gc_states();
            void refresh_counters();
            Element* mutable_element(const ElementHandle& element);
            RV apply_layout_subtree(const ElementHandle& element);
            RV apply_element_layout(const ElementHandle& element);
            Style& get_or_create_style(const Name& name);
            InteractionState& get_or_create_interaction(id_t id);
            void mark_subtree_interaction(id_t id, bool hovered, bool active, bool focused, bool clicked, bool double_clicked);
            void deliver_input_event(id_t id, const InputEvent& event);
            void record_draw_command(u32 layer_index, u32 element_index, const DrawCommand& command);
            bool point_hits_element(const Element& element, const Float2U& screen_position) const;
            bool element_can_focus(const Element& element) const;
            bool element_has_drag_source_type(const Element& element, const Name& payload_type) const;
            bool element_has_drag_target_type(const Element& element, const Name& payload_type) const;
            id_t focus_scope_of(id_t element_id) const;
            id_t scroll_target_of(id_t element_id) const;
            bool move_focus(bool reverse);
            bool move_focus_spatial(NavigationDirection direction);
            bool navigate(const NavigationRequest& request);
            FontDesc resolve_font(const Name& id) const;
            RectF to_screen_rect(u32 layer_index, const RectF& rect) const;
            RectF to_vg_rect(const RectF& screen_rect) const;
            const DragDropPayload* make_drag_drop_payload_view(const DragDropPayloadStorage& storage);
            void deliver_drag_drop_payload(const ElementHandle& target);
        };
    }
}
