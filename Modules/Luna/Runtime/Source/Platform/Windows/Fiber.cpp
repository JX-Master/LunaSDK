/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Fiber.cpp
* @author JXMaster
* @date 2026/2/10
*/
#include "../Fiber.hpp"
#include "ErrCode.hpp"
#include "../../../Platform/Windows/MiniWin.hpp"

namespace Luna
{
    namespace Platform
    {
        Result new_fiber(usize stack_size, void(*entry_func)(void* param), void* param, Fiber& out_fiber)
        {
            out_fiber.fiber = CreateFiber(stack_size, entry_func, param);
            if(out_fiber.fiber == NULL)
            {
                return translate_last_error(GetLastError());
            }
            return Result::success;
        }
        void delete_fiber(Fiber& fiber)
        {
            if(fiber.fiber)
            {
                DeleteFiber(fiber.fiber);
                fiber.fiber = NULL;
            }
        }
        Result convert_thread_to_fiber(Fiber& out_fiber)
        {
            out_fiber.fiber = ConvertThreadToFiber(NULL);
            if(out_fiber.fiber == NULL)
            {
                return translate_last_error(GetLastError());
            }
            return Result::success;
        }
        Result convert_fiber_to_thread()
        {
            if(!ConvertFiberToThread())
            {
                return translate_last_error(GetLastError());
            }
            return Result::success;
        }
        void switch_to_fiber(Fiber& fiber)
        {
            SwitchToFiber(fiber.fiber);
        }
        Result fls_alloc(void(*destructor)(void* ptr), opaque_t& out_handle)
        {
            DWORD index = FlsAlloc(destructor);
            if (index == FLS_OUT_OF_INDEXES)
            {
                DWORD err = GetLastError();
                return translate_last_error(err);
            }
            out_handle = (opaque_t)(usize)index;
            return Result::success;
        }
        void fls_free(opaque_t handle)
        {
            DWORD index = (DWORD)(usize)handle;
            FlsFree(index);
        }
        void fls_set(opaque_t handle, void* ptr)
        {
            if (!FlsSetValue((DWORD)(usize)handle, ptr))
            {
                lupanic_msg_always("TlsSetValue failed.");
            }
        }
        void* fls_get(opaque_t handle)
        {
            return FlsGetValue((DWORD)(usize)handle);
        }
    }
}
