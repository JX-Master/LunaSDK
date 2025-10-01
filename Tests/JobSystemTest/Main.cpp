/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Main.cpp
* @author JXMaster
* @date 2022/7/8
*/
#include <Luna/Runtime/Thread.hpp>
#include <Luna/JobSystem/JobSystem.hpp>
#include <Luna/Runtime/Time.hpp>
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
namespace Luna
{
    using namespace JobSystem;
    static void test_func_1(void* params)
    {
        sleep(1000);
        printf("Job executed in thread %lld\n", (u64)get_current_thread());
    }

    struct JobData
    {
        u32 recursive_depth;
    };

    static void test_func_2(void* params)
    {
        constexpr u32 TASKS_PER_JOB = 2;
        JobData* job_data = (JobData*)params;
        if (!job_data->recursive_depth)
        {
            sleep(100);
        }
        else
        {
            job_id_t ids[TASKS_PER_JOB];
            for (u32 i = 0; i < TASKS_PER_JOB; ++i)
            {
                JobData* subjob = (JobData*)new_job(test_func_2, sizeof(JobData), alignof(JobData), params);
                subjob->recursive_depth = job_data->recursive_depth - 1;
                ids[i] = submit_job(subjob);
            }
            for (u32 i = 0; i < TASKS_PER_JOB; ++i)
            {
                wait_job(ids[i]);
            }
        }
    }

    void job_system_test()
    {
        {
            u64 begin_time = get_ticks();
            constexpr usize N = 100;
            job_id_t jobs[N];
            for (usize i = 0; i < N; ++i)
            {
                void* job = new_job(test_func_1, 0, 0);
                jobs[i] = submit_job(job);
            }
            for(usize i = 0; i < N; ++i)
            {
                wait_job(jobs[i]);
            }
            u64 end_time = get_ticks();
            printf("Jon System Test 1: %u jobs finished in %f milliseconds.\n", (u32)N, (f64)(end_time - begin_time) / get_ticks_per_second() * 1000.0);
        }
        {
            constexpr u32 RECURSIVE_DEPTH = 10;
            u64 begin_time = get_ticks();
            JobData* root = (JobData*)new_job(test_func_2, sizeof(JobData), alignof(JobData));
            root->recursive_depth = RECURSIVE_DEPTH;
            job_id_t id = submit_job(root);
            wait_job(id);
            u64 end_time = get_ticks();
            printf("Jon System Test 1: %u levels of jobs finished in %f milliseconds.\n", RECURSIVE_DEPTH, (f64)(end_time - begin_time) / get_ticks_per_second() * 1000.0);
        }
    }
}

int main()
{
    Luna::init();
    lupanic_if_failed(Luna::add_module(Luna::module_job_system()));
    lupanic_if_failed(Luna::init_modules());
    Luna::job_system_test();
    Luna::close();
    return 0;
}