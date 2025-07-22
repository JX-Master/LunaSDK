/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Style.hpp
* @author JXMaster
* @date 2025/6/23
*/
#include "../Style.hpp"

namespace Luna
{
    namespace GUI
    {
        struct Style : virtual IStyle
        {
            lustruct("GUI::Style", "0110fc4e-adc5-4240-9d28-d8bd9cb8abb3");
            luiimpl();

            Ref<IStyle> m_parent;

            // Attribute values.
            HashMap<Name, Variant> m_values;

            virtual IStyle* get_parent() override
            {
                return m_parent;
            }
            
            virtual void set_parent(IStyle* parent) override
            {
                m_parent = parent;
            }

            virtual Variant get_value(const Name& name, const Variant& default_value, bool recursive, bool* found) override;

            virtual void set_value(const Name& name, const Variant& value) override;
        };
    }
}