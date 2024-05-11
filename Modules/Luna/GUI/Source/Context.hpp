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

            //! The root widget set by `reset`.
            Ref<Widget> m_new_root_widget;
            //! The root widget set by `update` from `m_new_root_widget` after widget diff.
            Ref<Widget> m_root_widget;
            Ref<WidgetBuildData> m_root_widget_build_data;
            Ref<VG::IFontAtlas> m_font_atlas;

            Context()
            {
                m_font_atlas = VG::new_font_atlas();
            }
            ~Context() {}
            virtual ContextIO& get_io() override
            {
                return m_io;
            }
            virtual void reset(Widget* root_widget) override;
            void diff_widget_tree();
            virtual RV update() override;
            virtual VG::IFontAtlas* get_font_altas() override
            {
                return m_font_atlas;
            }
            virtual RV render(VG::IShapeDrawList* draw_list) override;
        };
    }
}