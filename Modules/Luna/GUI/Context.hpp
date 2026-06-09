/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Context.hpp
* @author JXMaster
* @date 2026/5/22
*/
#pragma once
#include "Description.hpp"
#ifdef LUNA_GUI_ENABLE_DEBUG
#include "Debug.hpp"
#endif

namespace Luna
{
    namespace GUI
    {
        //! @addtogroup GUI GUI
        //! @{

        //! Runtime performance counters collected by one GUI context.
        //! @remark The counters describe the latest frame processed by the context. Timing values are in milliseconds.
        struct PerformanceCounters
        {
            //! Context generation of the frame represented by these counters.
            u64 frame_generation = 0;
            //! Number of nodes in the submitted description.
            u32 node_count = 0;
            //! Number of layers in the submitted description.
            u32 layer_count = 0;
            //! Number of nodes that participate in interaction.
            u32 interactive_node_count = 0;
            //! Number of input events consumed by submit.
            u32 input_event_count = 0;
            //! Number of layout passes executed in submit.
            u32 layout_pass_count = 0;
            //! Number of nodes measured by the layout system.
            u32 measured_node_count = 0;
            //! Number of nodes arranged by the layout system.
            u32 arranged_node_count = 0;
            //! Number of child subtrees skipped by layout because they were outside the scroll clip.
            u32 layout_clip_skipped_node_count = 0;
            //! Number of nodes visited for rendering.
            u32 rendered_node_count = 0;
            //! Number of node subtrees skipped by rendering because they were outside the clip.
            u32 render_clip_skipped_node_count = 0;
            //! Number of compiled VG draw calls emitted by GUI rendering.
            u32 render_draw_call_count = 0;
            //! Number of vertices in the compiled GUI VG vertex buffer.
            u32 render_vertex_count = 0;
            //! Number of indices in the compiled GUI VG index buffer.
            u32 render_index_count = 0;
            //! Total time spent in @ref IContext::submit.
            f64 submit_total_ms = 0.0;
            //! Time spent preparing popup, duplicate ID and pre-layout state in submit.
            f64 submit_prepare_ms = 0.0;
            //! Time spent in the first layout pass.
            f64 submit_layout_ms = 0.0;
            //! Time spent consuming input events.
            f64 submit_input_ms = 0.0;
            //! Time spent in the optional second layout pass after input changes.
            f64 submit_relayout_ms = 0.0;
            //! Time spent publishing item query states after input and layout.
            f64 submit_state_ms = 0.0;
            //! Total time spent in @ref IContext::render.
            f64 render_total_ms = 0.0;
            //! Time spent traversing nodes and recording GUI draw lists.
            f64 render_record_ms = 0.0;
            //! Time spent compiling VG draw commands.
            f64 render_compile_ms = 0.0;
            //! Time spent issuing VG renderer commands and submitting them to the command buffer.
            f64 render_submit_ms = 0.0;
        };

        //! @interface IContext
        //! Owns GUI frame building, input routing, state storage, layout, render list generation and rendering.
        //! @remark Widget APIs take an explicit context pointer as their first argument. This avoids global GUI state and
        //! allows multiple contexts to be built from different threads or hosts.
        struct IContext : virtual Interface
        {
            luiid("{E58F6F6C-48A9-42AB-86F3-898419C207BC}");

            //! Begins a new GUI frame.
            //! @param[in] desc The logical screen, framebuffer and timing information for this frame.
            //! @remark This resets per-frame build state and expires states whose lifetime requires frame cleanup.
            virtual void begin_frame(const FrameDesc& desc) = 0;

            //! Queues one input event for the current frame.
            //! @param[in] event The input event in GUI screen coordinates.
            //! @remark Queued events are consumed by @ref submit.
            virtual void add_input_event(const InputEvent& event) = 0;

            //! Queues multiple input events for the current frame.
            //! @param[in] events The events in the order they should be processed.
            virtual void add_input_events(Span<const InputEvent> events) = 0;

            //! Begins building a new GUI layer.
            //! @param[in] id The stable layer identifier. The same layer ID should be reused across frames.
            //! @param[in] screen_position The layer top-left position in screen coordinates.
            //! @remark The first node added after this call becomes the root node of the layer.
            virtual void push_layer(id_t id, const Float2U& screen_position = Float2U(0.0f)) = 0;

            //! Ends the current GUI layer.
            virtual void pop_layer() = 0;

            //! Adds a node to the description being built.
            //! @param[in] node The node object to add.
            //! @param[in] label Optional label used for text and default ID generation.
            //! @param[in] interactive Whether the node should participate in item hit testing.
            //! @return Returns the handle used to query item state for the node.
            virtual ItemHandle add_node(Ref<Node> node, const c8* label = nullptr, bool interactive = false) = 0;

            //! Gets the current context generation.
            //! @return Returns the generation value used to validate item handles.
            virtual u64 generation() const = 0;

            //! Gets a state object by ID.
            //! @param[in] id The state identifier.
            //! @return Returns the boxed state object, or `nullptr` when no state exists.
            virtual object_t get_state(id_t id) = 0;

            //! Sets or refreshes a state object.
            //! @param[in] id The state identifier.
            //! @param[in] data The boxed state object.
            //! @param[in] lifetime The lifetime used for automatic cleanup.
            //! @return Returns success or failure code.
            virtual RV set_state(id_t id, object_t data, StateLifetime lifetime = StateLifetime::next_frame) = 0;

            //! Clears one state object immediately.
            //! @param[in] id The state identifier.
            virtual void clear_state(id_t id) = 0;

            //! Defines a style if it does not already exist.
            //! @param[in] name The style name.
            //! @param[in] parent Optional parent style name.
            virtual void define_style(const Name& name, const Name& parent = Name()) = 0;

            //! Changes the parent style of an existing style.
            //! @param[in] name The style name.
            //! @param[in] parent The new parent style name, or an empty name for no parent.
            virtual void set_style_parent(const Name& name, const Name& parent) = 0;

            //! Sets or overrides one style entry.
            //! @param[in] style The style name.
            //! @param[in] entry The style entry name.
            //! @param[in] value The entry value.
            virtual void set_style_value(const Name& style, const Name& entry, const StyleValue& value) = 0;

            //! Removes a local style entry so that the value is inherited from the parent style.
            //! @param[in] style The style name.
            //! @param[in] entry The style entry name.
            virtual void inherit_style_entry(const Name& style, const Name& entry) = 0;

            //! Marks a style entry as explicitly unset, hiding an inherited value.
            //! @param[in] style The style name.
            //! @param[in] entry The style entry name.
            virtual void unset_style_entry(const Name& style, const Name& entry) = 0;

            //! Resolves one style entry through the style inheritance chain.
            //! @param[in] style The style name.
            //! @param[in] entry The style entry name.
            //! @param[in] default_value The fallback value if the entry is not found or is unset.
            //! @return Returns the resolved style value.
            virtual StyleValue get_style_value(const Name& style, const Name& entry, const StyleValue& default_value) = 0;

            //! Pushes a style onto the build style stack.
            //! @param[in] style The style name to bind to subsequently created nodes.
            virtual void push_style(const Name& style) = 0;

            //! Pops the current build style.
            virtual void pop_style() = 0;

            //! Registers a font file for style-based text rendering.
            //! @param[in] id The unique font ID used by style entries.
            //! @param[in] font The font file to register. The context retains this object.
            //! @param[in] font_index The font face index inside `font`.
            //! @return Returns success or failure code.
            virtual RV register_font(const Name& id, Font::IFontFile* font, u32 font_index = 0) = 0;

            //! Gets a registered font by ID.
            //! @param[in] id The font ID.
            //! @return Returns the font descriptor, or an empty descriptor when the font is not registered.
            virtual FontDesc get_font(const Name& id) = 0;

            //! Assigns a render proxy to the next node created by widget APIs.
            //! @param[in] proxy The render proxy callbacks.
            virtual void set_next_item_render_proxy(const RenderProxyDesc& proxy) = 0;

            //! Assigns enabled state to the next node created by widget APIs.
            //! @param[in] enabled Whether the next item accepts interaction and normal interactive visuals.
            virtual void set_next_item_enabled(bool enabled) = 0;

            //! Pushes an enabled state onto the build stack.
            //! @param[in] enabled Whether subsequently created items are enabled while this state is on the stack.
            //! @remark The effective item enabled state is the logical AND of the enabled stack and any next-item override.
            virtual void push_enabled(bool enabled) = 0;

            //! Pops the current enabled state from the build stack.
            virtual void pop_enabled() = 0;

            //! Finishes immediate-mode building and returns the description object for the frame.
            //! @return Returns the completed description, or a failure code.
            virtual R<Description> end_build() = 0;

            //! Submits a GUI description to update layout, input state and render data.
            //! @param[in] desc The description generated for this frame.
            //! @return Returns success or failure code.
            //! @remark Input events queued before this call become visible through item state queries after this call.
            virtual RV submit(const Description& desc) = 0;

            //! Sets clipboard callbacks used by editable text widgets.
            //! @param[in] io Clipboard callback table.
            virtual void set_clipboard_io(const ClipboardIO& io) = 0;

            //! Gets the active text input state for host IME and virtual keyboard integration.
            //! @return Returns the current text input state.
            virtual TextInputState get_text_input_state() = 0;

            //! Gets performance counters collected for the latest processed GUI frame.
            //! @return Returns a copy of the current performance counters.
            virtual PerformanceCounters get_performance_counters() = 0;
#ifdef LUNA_GUI_ENABLE_DEBUG
            //! Dumps debug information for the latest submitted frame.
            //! @return Returns a serializable debug information snapshot.
            virtual R<DebugInfo> dump_debug_info() = 0;
#endif
            //! Renders the latest submitted GUI into the specified render target.
            //! @param[in] cmdbuf The command buffer used to submit rendering commands.
            //! @param[in] render_target The render target to draw into.
            //! @return Returns success or failure code.
            virtual RV render(RHI::ICommandBuffer* cmdbuf, RHI::ITexture* render_target) = 0;
        };

        //! Creates a new GUI context.
        //! @param[in] device Optional RHI device. Passing `nullptr` lets the implementation use its default device path.
        //! @return Returns the created context.
        LUNA_GUI_API Ref<IContext> new_context(RHI::IDevice* device = nullptr);

        //! Gets the GUI module object.
        //! @return Returns the GUI module object.
        LUNA_GUI_API Module* module_gui();

        //! @}
    }
}
