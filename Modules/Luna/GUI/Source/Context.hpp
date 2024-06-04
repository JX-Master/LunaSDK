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

namespace Luna
{
    namespace GUI
    {
        struct Context : public IContext
        {
            lustruct("GUI::Context", "{2ee81356-fb85-4fea-ad8b-578635de5c6a}");
            luiimpl();

            ContextIO m_io;

            Vector<widget_id_t> m_id_stack;

            // Widget build context.
            Ref<Widget> m_current_widget;
            Vector<Ref<Widget>> m_widget_stack;
            
            // Widget update context.
            Ref<Widget> m_root_widget;

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

            Context()
            {
                m_font_atlas = VG::new_font_atlas();
            }
            ~Context() {}
            virtual ContextIO& get_io() override
            {
                return m_io;
            }
            virtual void begin_frame() override;
            virtual void end_frame() override;
            virtual void push_id(const Name& name_id) override
            {
                m_id_stack.push_back(get_id(name_id));
            }
            virtual void push_id(const c8* str_id, usize str_len) override
            {
                m_id_stack.push_back(get_id(str_id, str_len));
            }
            virtual void push_id(const void* ptr_id) override
            {
                m_id_stack.push_back(get_id(ptr_id));
            }
            virtual void push_id(i32 int_id) override
            {
                m_id_stack.push_back(get_id(int_id));
            }
            virtual void pop_id() override
            {
                m_id_stack.pop_back();
            }
            //! Generates widget ID based on the current widget ID stack.
            virtual widget_id_t get_id() override
            {
                widget_id_t id = m_id_stack.empty() ? 0 : m_id_stack.back();
                return id;
            }
            virtual widget_id_t get_id(const Name& name_id) override
            {
                name_id_t id = name_id.id();
                return memhash32(&id, sizeof(name_id_t), get_id());
            }
            virtual widget_id_t get_id(const c8* str_id, usize str_len) override
            {
                if(str_len == USIZE_MAX) str_len = strlen(str_id);
                return memhash32(str_id, str_len, get_id());
            }
            virtual widget_id_t get_id(const void* ptr_id) override
            {
                return memhash32(&ptr_id, sizeof(void*), get_id());
            }
            virtual widget_id_t get_id(i32 int_id) override
            {
                return memhash32(&int_id, sizeof(i32), get_id());
            }
            virtual Widget* get_current_widget() override
            {
                return m_current_widget;
            }
            virtual void add_widget(Widget* widget) override;
            virtual void push_widget(Widget* widget) override
            {
                m_widget_stack.push_back(widget);
            }
            virtual void pop_widget() override
            {
                m_widget_stack.pop_back();
            }
            virtual object_t get_widget_state(widget_id_t id) override;
            virtual void set_widget_state(widget_id_t id, object_t state, WidgetStateLifetime lifetime) override;
            virtual RV update() override;
            virtual VG::IFontAtlas* get_font_altas() override
            {
                return m_font_atlas;
            }
            virtual RV render(VG::IShapeDrawList* draw_list) override;
        };
    }
}