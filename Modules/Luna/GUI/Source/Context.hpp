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

namespace Luna
{
    namespace GUI
    {
        enum class WidgetType : u8
        {
            container = 0,
            text = 1
        };

        struct Widget;
        struct Widget
        {
            WidgetType m_type;
            Widget* m_parent = nullptr;
            Vector<Widget*> m_children;

            u32 m_background_color = 0xFFFFFFFF;
            u32 m_border_color = 0xFFFFFFFF;
            OffsetRectF m_anthor = OffsetRectF(0, 0, 1, 1);
            OffsetRectF m_rect = OffsetRectF(0, 0, 0, 0);

            // Generated data.
            f32 m_min_x;
            f32 m_min_y;
            f32 m_max_x;
            f32 m_max_y;

            Widget(WidgetType type = WidgetType::container) :
                m_type(type) {}
            virtual ~Widget() {}
        };

        struct TextWidget : public Widget
        {
            Name m_text;
            u32 m_text_color = 0xFFFFFFFF;
            f32 m_text_size = 16.0f;

            // Generated data.
            VG::TextArrangeResult m_arrange_result;
            Vector<VG::TextArrangeSection> m_text_arrange_sections;

            TextWidget() :
                Widget(WidgetType::text) {}
        };

        struct Context : public IContext
        {
            lustruct("GUI::Context", "{2ee81356-fb85-4fea-ad8b-578635de5c6a}");
            luiimpl();

            ContextIO m_io;
            bool m_dirty;

            Vector<UniquePtr<Widget>> m_widgets;
            Widget* m_root_widget;
            Widget* m_current_widget;
            Vector<Widget*> m_widget_stack;

            Ref<VG::IFontAtlas> m_font_atlas;

            Context() :
                m_dirty(true),
                m_root_widget(nullptr),
                m_current_widget(nullptr)
            {
                m_font_atlas = VG::new_font_atlas();
            }

            Widget* add_widget(WidgetType type)
            {
                switch(type)
                {
                    case WidgetType::container:
                        m_widgets.emplace_back(memnew<Widget>());
                        break;
                    case WidgetType::text:
                        m_widgets.emplace_back(memnew<TextWidget>());
                        break;
                }
                m_current_widget = m_widgets.back().get();
                if(!m_widget_stack.empty())
                {
                    m_widget_stack.back()->m_children.push_back(m_current_widget);
                    m_current_widget->m_parent = m_widget_stack.back();
                }
                return m_current_widget;
            }

            virtual ContextIO& get_io() override
            {
                return m_io;
            }
            virtual RV reset(IWidgetList* widget_list) override;
            virtual RV update() override;
            virtual bool is_dirty() override
            {
                return m_dirty;
            }
            virtual void set_dirty() override
            {
                m_dirty = true;
            }
            virtual RV render(VG::IShapeDrawList* draw_list) override;
        };
    }
}