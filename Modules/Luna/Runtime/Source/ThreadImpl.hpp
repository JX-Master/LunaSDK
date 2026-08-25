/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Thread.hpp
* @author JXMaster
* @date 2018/12/21
*/
#pragma once
#include "../Thread.hpp"
#include "../Fiber.hpp"
#include "Platform/Thread.hpp"
#include "ErrorImpl.hpp"
#include "ThreadImpl.generated.hpp"
namespace Luna
{
    struct [[luna::struct("{3a99cfa4-5857-4135-82dc-f0dfb4ac5272}")]] ThreadBase : IThread
    {
        luiimpl();

        Platform::Thread m_thread;
        Ref<IFiber> m_native_fiber;
        Ref<IFiber> m_current_fiber;

        virtual ~ThreadBase() {}
        
        virtual RV set_priority(ThreadPriority priority)  override
        {
            return encode_platform_result(Platform::set_thread_priority(m_thread, priority));
        }
        virtual IFiber* get_fiber() override
        {
            return m_native_fiber.get();
        }
    };
    struct [[luna::struct("{a29d30a1-e572-4e61-9e3e-5083b3e0ff64}")]] Thread : ThreadBase
    {
        luiimpl();

        void(*m_entry)(void*);
        void* m_params;

        virtual void wait() override
        {
            Platform::wait_thread(m_thread);
        }
        virtual bool try_wait() override
        {
            return Platform::try_wait_thread(m_thread);
        }
        ~Thread()
        {
            if (m_thread.valid())
            {
                wait();
                Platform::detach_thread(m_thread);
            }
        }
    };
    struct [[luna::struct("{384494c9-298b-47b8-af1f-83e26ecd429a}")]] MainThread : ThreadBase
    {
        luiimpl();

        virtual void wait() override
        {
            lupanic_msg_always("The main thread cannot be waited, since it never returns.");
        }
        virtual bool try_wait() override
        {
            // The main thread cannot be waited.
            return false;
        }
    };
    RV thread_init();
    void thread_close();
}
