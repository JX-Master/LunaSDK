/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Init.cpp
* @author JXMaster
* @date 2020/12/9
*/
#include "../Init.hpp"
#include "../../../Platform/Windows/MiniWin.hpp"

namespace Luna
{
    namespace Platform
    {
        void time_init();
        Result thread_init();
        void thread_close();
        void debug_init();
        void debug_close();

        Result init()
        {
            Result r;
            debug_init();
            time_init();
            r = thread_init();
            if(r != Result::success)
            {
                debug_close();
                return r;
            }
            return Result::success;
        }

        void close() 
        {
            thread_close();
            debug_close();
        }
    }
}
