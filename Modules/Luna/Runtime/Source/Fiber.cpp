/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Fiber.cpp
* @author JXMaster
* @date 2026/2/17
*/
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "Thread.hpp"
#include "Fiber.hpp"

namespace Luna
{
    Fiber::~Fiber()
    {
        if(m_should_delete)
        {
            Platform::delete_fiber(m_fiber);
        }
    }
    LUNA_RUNTIME_API R<Ref<IFiber>> new_fiber(usize stack_size, void(*entry_func)(void* param), void* param)
    {
        luassert(entry_func && stack_size);
        Ref<Fiber> f = new_object<Fiber>();
        auto r = Platform::new_fiber(stack_size, entry_func, param, f->m_fiber);
        if(r != Platform::Result::success) return encode_platform_result(r).errcode();
        f->m_should_delete = true;
        return Ref<IFiber>(f);
    }
    LUNA_RUNTIME_API R<Ref<IFiber>> convert_thread_to_fiber()
    {
        ThreadBase* t = cast_object<ThreadBase>(get_current_thread()->get_object());
        if(t->m_native_fiber) return t->m_native_fiber;
        Ref<Fiber> f = new_object<Fiber>();
        auto r = Platform::convert_thread_to_fiber(f->m_fiber);
        if(r != Platform::Result::success) return encode_platform_result(r).errcode();
        t->m_native_fiber = f;
        t->m_current_fiber = f;
        return Ref<IFiber>(f);
    }
    LUNA_RUNTIME_API RV convert_fiber_to_thread()
    {
        auto r = Platform::convert_fiber_to_thread();
        if(r != Platform::Result::success) return encode_platform_result(r);
        ThreadBase* t = cast_object<ThreadBase>(get_current_thread()->get_object());
        t->m_native_fiber.reset();
        t->m_current_fiber.reset();
        return ok;
    }
    LUNA_RUNTIME_API void switch_to_fiber(IFiber* fiber)
    {
        ThreadBase* t = cast_object<ThreadBase>(get_current_thread()->get_object());
        t->m_current_fiber = fiber;
        Fiber* f = cast_object<Fiber>(fiber->get_object());
        Platform::switch_to_fiber(f->m_fiber);
    }
    LUNA_RUNTIME_API IFiber* get_current_fiber()
    {
        ThreadBase* t = cast_object<ThreadBase>(get_current_thread()->get_object());
        return t->m_current_fiber.get();
    }
    LUNA_RUNTIME_API R<opaque_t> fls_alloc(void(*destructor)(void* ptr))
    {
        opaque_t handle;
        auto r = Platform::fls_alloc(destructor, handle);
        if(r != Platform::Result::success)
        {
            return encode_platform_result(r).errcode();
        }
        return handle;
    }
    LUNA_RUNTIME_API void fls_free(opaque_t handle)
    {
        Platform::fls_free(handle);
    }
    LUNA_RUNTIME_API void fls_set(opaque_t handle, void* ptr)
    {
        Platform::fls_set(handle, ptr);
    }
    LUNA_RUNTIME_API void* fls_get(opaque_t handle)
    {
        return Platform::fls_get(handle);
    }
}