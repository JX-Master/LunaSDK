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

            //! Assigns a render proxy to the next node created by widget APIs.
            //! @param[in] proxy The render proxy callbacks.
            virtual void set_next_item_render_proxy(const RenderProxyDesc& proxy) = 0;

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
