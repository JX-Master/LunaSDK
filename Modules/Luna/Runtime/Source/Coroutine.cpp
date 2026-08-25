/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Coroutine.cpp
* @author JXMaster
* @date 2026/2/14
*/
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "CoroutineImpl.hpp"
#include "ErrorImpl.hpp"
#include "Platform/Thread.hpp"

namespace Luna
{
    opaque_t g_current_coroutine_tls;

    RV coroutine_init()
    {
        auto r = Platform::tls_alloc(g_current_coroutine_tls);
        if(r != Platform::Result::success) return encode_platform_result(r);
        return ok;
    }
    void coroutine_close()
    {
        Platform::tls_free(g_current_coroutine_tls);
    }
    static void coroutine_entry(void* params)
    {
        Coroutine* ctx = (Coroutine*)params;
        ctx->m_entry_func(ctx->m_param);
        while(true)
        {
            yield_coroutine();
        }
    }
    LUNA_RUNTIME_API ICoroutine* get_current_coroutine()
    {
        Coroutine* coro = (Coroutine*)Platform::tls_get(g_current_coroutine_tls);
        return coro;
    }
    LUNA_RUNTIME_API R<Ref<ICoroutine>> new_coroutine(usize stack_size, void(*entry_func)(void* param), void* param)
    {
        Ref<ICoroutine> ret;
        lutry
        {
            Ref<Coroutine> ctx = new_object<Coroutine>();
            ctx->m_entry_func = entry_func;
            ctx->m_param = param;
            lulet(fiber, new_fiber(stack_size, coroutine_entry, ctx.get()));
            ctx->m_fiber = fiber;
            ret = ctx;
        }
        lucatchret;
        return ret;
    }
    LUNA_RUNTIME_API void resume_coroutine(ICoroutine* coroutine)
    {
        Coroutine* coro = cast_object<Coroutine>(coroutine->get_object());
        Coroutine* parent_coroutine = (Coroutine*)Platform::tls_get(g_current_coroutine_tls);
        coro->m_parent = parent_coroutine ? parent_coroutine->get_fiber() : get_current_thread()->get_fiber();
        luassert_msg_always(coro->m_parent, "resume_coroutine must be called in a fiber context");
        Platform::tls_set(g_current_coroutine_tls, coro);
        switch_to_fiber(coro->m_fiber);
        Platform::tls_set(g_current_coroutine_tls, parent_coroutine);
    }
    LUNA_RUNTIME_API void yield_coroutine()
    {
        Coroutine* coro = (Coroutine*)Platform::tls_get(g_current_coroutine_tls);
        if(!coro) [[unlikely]]
        {
            lupanic_msg_always("yield_coroutine is called from a non-coroutine context");
        }
        IFiber* parent = coro->m_parent;
        coro->m_parent = nullptr;
        switch_to_fiber(parent);
    }
}
