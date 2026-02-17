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
#include "OS.hpp"
namespace Luna
{
    struct ThreadBase : IThread
    {
        lustruct("ThreadBase", "{3a99cfa4-5857-4135-82dc-f0dfb4ac5272}");
        luiimpl();

        opaque_t m_handle;
        Ref<IFiber> m_native_fiber;
        Ref<IFiber> m_current_fiber;

        ThreadBase() :
            m_handle(nullptr) {}

        virtual ~ThreadBase() {}
        
        virtual void set_priority(ThreadPriority priority)  override
        {
            OS::set_thread_priority(m_handle, priority);
        }
        virtual IFiber* get_fiber() override
        {
            return m_native_fiber.get();
        }
    };
    struct Thread : ThreadBase
    {
        lustruct("Thread", "{a29d30a1-e572-4e61-9e3e-5083b3e0ff64}");
        luiimpl();

        void(*m_entry)(void*);
        void* m_params;

        virtual void wait() override
        {
            OS::wait_thread(m_handle);
        }
        virtual bool try_wait() override
        {
            return OS::try_wait_thread(m_handle);
        }
        ~Thread()
        {
            if (m_handle)
            {
                wait();
                OS::detach_thread(m_handle);
            }
        }
    };
    struct MainThread : ThreadBase
    {
        lustruct("MainThread", "{384494c9-298b-47b8-af1f-83e26ecd429a}");
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
    void thread_init();
    void thread_close();
}
