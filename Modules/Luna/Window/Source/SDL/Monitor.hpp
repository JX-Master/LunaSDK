/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Monitor.hpp
* @author JXMaster
* @date 2024/6/16
*/
#include "../../Monitor.hpp"
#include <Luna/Runtime/Result.hpp>

namespace Luna
{
    namespace Window
    {
        struct Monitor
        {
            Name m_name;
            u32 m_index;
            bool m_disconnected = false;
        };

        RV monitor_init();
        void monitor_close();

        RV refresh_monitor_list();
        void dispatch_monitor_event(monitor_t monitor, const MonitorEvent& e);
    }
}