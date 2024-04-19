/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file WidgetList.hpp
* @author JXMaster
* @date 2024/3/29
*/
#include "../WidgetList.hpp"

namespace Luna
{
    namespace GUI
    {
        struct WidgetList : IWidgetList
        {
            lustruct("GUI::WidgetList", "{978cad33-41b8-4d26-b450-3829fd30c55b}");
            luiimpl();

            Vector<u32> m_widget_buffer;
            Vector<Name> m_texts;

            WidgetList()
            {
                reset();
            }
            virtual void reset() override
            {
                m_widget_buffer.clear();
                m_texts.clear();
            }
            virtual Vector<u32>& get_widget_buffer() override
            {
                return m_widget_buffer;
            }
            virtual u32 add_text(const Name& text) override
            {
                u32 i = (u32)m_texts.size();
                m_texts.push_back(text);
                return i;
            }
            virtual Name get_text(u32 index) override
            {
                if(index >= m_texts.size()) return Name();
                return m_texts[index];
            }
        };
    }
}