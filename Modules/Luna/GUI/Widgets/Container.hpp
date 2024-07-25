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
        //! @interface IContainer
        //! Represents one widget that can contain child widgets.
        //! Implement this interface so that the system can recognize one widget as one container.
        struct IContainer : virtual IWidget
        {
            luiid("15a3b87a-df22-429f-acc8-214e585210f6");

            //! Adds one child widget to this container.
            virtual void add_child(IWidget* child) = 0;

            //! Gets a list of child widgets of this container.
            //! @param[out] out_children The array to write child widgets to.
            //! Child widgets will be pushed to the end of this array, existing elements in the 
            //! array are not changed.
            virtual void get_children(Vector<IWidget*>& out_children) = 0;

            //! Gets the number of child widgets of tihs container.
            //! @return Returns the number of child widgets of tihs container.
            virtual usize get_num_children() = 0;
        };
    }
}