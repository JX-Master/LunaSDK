/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file JobContext.hpp
* @author JXMaster
* @date 2026/2/14
*/
#pragma once
#include <Luna/Runtime/Coroutine.hpp>
#include "../JobSystem.hpp"

namespace Luna
{
    namespace JobSystem
    {
        struct JobInfo
        {
            job_id_t m_id = INVALID_JOB_ID;
            void (*m_func)(void* params) = nullptr;
            void* m_params = nullptr;
            bool m_finished = false;
        };

        // The job executing context.
        // This will be reused across different jobs.
        struct JobContext
        {
            Ref<ICoroutine> m_coroutine;
            Vector<job_id_t> m_wait_jobs;
            JobInfo* m_job = nullptr;

            bool is_job_ready_to_resume() const
            {
                for(job_id_t job : m_wait_jobs)
                {
                    if(!is_job_finished(job))
                    {
                        return false;
                    }
                }
                return true;
            }
        };

        JobContext* allocate_job_context();
        void free_job_context(JobContext* ctx);
        void clean_up_job_contexts();
    }
}