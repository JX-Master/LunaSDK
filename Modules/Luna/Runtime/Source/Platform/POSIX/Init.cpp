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
        void file_init();
        void std_io_init();
        void std_io_close();

        void init()
        {
            time_init();
            thread_init();
            file_init();
            std_io_init();
        }

        void close()
        {
            std_io_close();
        }
    }
}