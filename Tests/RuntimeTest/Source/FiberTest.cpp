/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FiberTest.hpp
* @author JXMaster
* @date 2026/2/19
*/
#include "TestCommon.hpp"
#include <Luna/Runtime/Thread.hpp>
#include <Luna/Runtime/Fiber.hpp>
#include <Luna/Runtime/UniquePtr.hpp>

namespace Luna
{
    struct ThreadParams
    {
        String text;
    };
    static void thread_entry(void* params)
    {
        ThreadParams* p = (ThreadParams*)params;
        sleep(100);
        printf("Thread Test: ID: %llu, Text:%s\n", (u64)get_current_thread_id(), p->text.c_str());
        sleep(100);
    }

    struct FiberParams
    {
        IFiber* main_fiber;
        String text;
    };

    static void fiber_entry(void* params)
    {
        FiberParams* p = (FiberParams*)params;
        sleep(100);
        printf("Fiber %s running on thread: %llu", p->text.c_str(), (u64)get_current_thread_id());
        sleep(100);
        switch_to_fiber(p->main_fiber);
    }

    void fiber_test()
    {
        // Fiber in main thread.
        {
            auto main_fiber = convert_thread_to_fiber();
            lutest(succeeded(main_fiber));

            Vector<Ref<IFiber>> fibers;
            Vector<UniquePtr<FiberParams>> fiber_params;

            for(usize i = 0; i < 10; ++i)
            {
                UniquePtr<FiberParams> param(memnew<FiberParams>());
                param->main_fiber = main_fiber.get();
                strprintf(param->text, "%u", (u32)i);
                auto fiber = new_fiber(128_kb, fiber_entry, param.get());
                lutest(succeeded(fiber));
                fibers.push_back(fiber.get());
                fiber_params.push_back(move(param));
            }

            for(auto& fiber : fibers)
            {
                switch_to_fiber(fiber.get());
            }

            lutest(succeeded(convert_fiber_to_thread()));
        }
        // Fiber in user thread.
        {
            
        }
        
    }
}