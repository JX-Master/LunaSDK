/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Fiber.cpp
* @author JXMaster
* @date 2026/2/10
*/
#include "../../OS.hpp"
#include "ErrCode.hpp"
#include "../../../Platform/Windows/MiniWin.hpp"

namespace Luna
{
    namespace OS
    {
        RV new_fiber(usize stack_size, void(*entry_func)(void* param), void* param, FiberContext& out_fiber)
        {
            out_fiber.fiber = CreateFiber(stack_size, entry_func, param);
            if(out_fiber.fiber == NULL)
            {
                return translate_last_error(GetLastError());
            }
            return ok;
        }

        void delete_fiber(FiberContext& fiber)
        {
            if(fiber.fiber)
            {
                DeleteFiber(fiber.fiber);
                fiber.fiber = NULL;
            }
        }

        RV convert_thread_to_fiber(FiberContext& out_fiber)
        {
            out_fiber.fiber = ConvertThreadToFiber(NULL);
            if(out_fiber.fiber == NULL)
            {
                return translate_last_error(GetLastError());
            }
            return ok;
        }

        RV convert_fiber_to_thread()
        {
            if(!ConvertFiberToThread())
            {
                return translate_last_error(GetLastError());
            }
            return ok;
        }

        void switch_to_fiber(FiberContext& fiber)
        {
            SwitchToFiber(fiber.fiber);
        }
    }
}