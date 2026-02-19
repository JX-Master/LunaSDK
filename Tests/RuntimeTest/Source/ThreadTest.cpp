/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ThreadTest.hpp
* @author JXMaster
* @date 2026/2/19
*/
#include "TestCommon.hpp"
#include <Luna/Runtime/Thread.hpp>
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
    void thread_test()
    {
        Vector<UniquePtr<ThreadParams>> params;
        Vector<Ref<IThread>> threads;
        for(usize i = 0; i < 10; ++i)
        {
            UniquePtr<ThreadParams> param(memnew<ThreadParams>());
            strprintf(param->text, "Thread %u", (u32)i);
            auto t = new_thread(thread_entry, param.get());
            lutest(succeeded(t));
            threads.push_back(t.get());
            params.push_back(move(param));
        }
        // Wait for all threads.
        threads.clear();
    }
}