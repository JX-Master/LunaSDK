/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Init.cpp
* @author JXMaster
* @date 2021/4/25
 */
#include "../Init.hpp"
#include <sys/types.h>
#ifdef LUNA_PLATFORM_MACOS
#include <sys/sysctl.h>
#else
#include <unistd.h>
#endif

namespace Luna
{
    namespace Platform
    {
        void time_init();
        void thread_init();
        Result fiber_init();
        void std_io_init();
        void std_io_close();
        void fiber_close();

        Result init()
        {
            time_init();
            thread_init();
            auto r = fiber_init();
            if(r != Result::success)
            {
                return r;
            }
            std_io_init();
            return Result::success;
        }

        void close()
        {
            fiber_close();
            std_io_close();
        }
    }
}