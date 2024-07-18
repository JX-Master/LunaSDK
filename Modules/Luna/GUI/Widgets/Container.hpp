/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Container.hpp
* @author JXMaster
* @date 2024/7/18
*/
#pragma once
#include "../Widget.hpp"

namespace Luna
{
    namespace GUI
    {
        struct IContainer : virtual IWidget
        {
            luiid("15a3b87a-df22-429f-acc8-214e585210f6");

            //! Adds one child widget to this container.
            virtual void add_child(IWidget* child) = 0;

            //! Gets a list of child widgets of this container.
            //! @remark This is a generic method of getting children. 
            //! The implementation may provide a better way of accessing child widgets.
            virtual Array<IWidget*> get_children() = 0;
        };
    }
}