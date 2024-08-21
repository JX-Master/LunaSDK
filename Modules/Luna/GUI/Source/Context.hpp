/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Context.hpp
* @author JXMaster
* @date 2024/3/29
*/
#include "../Context.hpp"
#include <Luna/Runtime/UniquePtr.hpp>
#include <Luna/VG/TextArranger.hpp>
#include "../Widget.hpp"
#include <Luna/Runtime/RingDeque.hpp>
#include "../Event.hpp"

namespace Luna
{
    namespace GUI
    {
        struct Context : public IContext
        {
            lustruct("GUI::Context", "{2ee81356-fb85-4fea-ad8b-578635de5c6a}");
            luiimpl();

            ContextIO m_io;

            // Widget update context.
            Ref<IWidget> m_root_widget;

            // Render context.
            Ref<VG::IFontAtlas> m_font_atlas;

            // Widget state registry.
            struct WidgetStateEntry
            {
                ObjRef state;
                WidgetStateLifetime lifetime;

                WidgetStateEntry(const ObjRef& s, WidgetStateLifetime l) :
                    state(s),
                    lifetime(l) {}
            };
            HashMap<widget_id_t, WidgetStateEntry> m_widget_state_reg;

            // Event queue.
            RingDeque<ObjRef> m_event_queue;
            Vector<Pair<IWidget*, typeinfo_t>> m_event_capture_stack;

            Context()
            {
                memzero(&m_io);
                m_font_atlas = VG::new_font_atlas();
            }
            ~Context() {}
            virtual ContextIO& get_io() override
            {
                return m_io;
            }
            virtual IWidget* get_widget() override
            {
                return m_root_widget;
            }
            virtual void set_widget(IWidget* root_widget) override;
            virtual object_t get_widget_state(widget_id_t id) override;
            virtual void set_widget_state(widget_id_t id, object_t state, WidgetStateLifetime lifetime) override;
            virtual void push_event(object_t event) override
            {
                m_event_queue.push_back(ObjRef(event));
            }
            virtual void capture_event(IWidget* widget, typeinfo_t event_type) override
            {
                m_event_capture_stack.push_back(make_pair(widget, event_type));
            }
            RV dispatch_mouse_event(MouseEvent* event, bool& handled);
            RV dispatch_event(object_t event);
            virtual RV update() override;
            virtual VG::IFontAtlas* get_font_altas() override
            {
                return m_font_atlas;
            }
            virtual RV render(IDrawList* draw_list, IDrawList* overlay_draw_list) override;
        };
    }
}