/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file JobContext.cpp
* @author JXMaster
* @date 2026/2/14
*/
#include "JobContext.hpp"
#include <Luna/Runtime/SpinLock.hpp>

namespace Luna
{
    namespace JobSystem
    {
        static SpinLock g_job_contexts_lock;
        static Vector<JobContext*> g_job_contexts;
        static Vector<JobContext*> g_free_job_contexts;

        constexpr usize JOB_FIBER_STACK_SIZE = 256_kb;

        void job_entry(void* params);

        JobContext* allocate_job_context()
        {
            LockGuard guard(g_job_contexts_lock);
            if(g_free_job_contexts.empty())
            {
                // Create new job context.
                JobContext* ctx = memnew<JobContext>();
                auto coroutine = new_coroutine(JOB_FIBER_STACK_SIZE, job_entry, ctx);
                if (failed(coroutine)) [[unlikely]]
                {
                    lupanic_msg_always("Failed to create coroutine context for job");
                }
                ctx->m_coroutine = coroutine.get();
                g_job_contexts.push_back(ctx);
                return ctx;
            }
            JobContext* ctx = g_free_job_contexts.back();
            g_free_job_contexts.pop_back();
            return ctx;
        }
        void free_job_context(JobContext* ctx)
        {
            LockGuard guard(g_job_contexts_lock);
            g_free_job_contexts.push_back(ctx);
        }
        void clean_up_job_contexts()
        {
            for(JobContext* job : g_job_contexts)
            {
                memdelete(job);
            }
            g_job_contexts.clear();
            g_job_contexts.shrink_to_fit();
        }
    }
}