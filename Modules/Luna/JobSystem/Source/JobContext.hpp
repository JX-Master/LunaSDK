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
            void (*m_func)(IJobScheduler* scheduler, void* params) = nullptr;
            void* m_params = nullptr;
            bool m_finished = false;
        };

        struct JobScheduler;
        // The job executing context.
        // This will be reused across different jobs.
        struct JobContext
        {
            JobScheduler* m_owner;
            Ref<ICoroutine> m_coroutine;
            Vector<job_id_t> m_wait_jobs;
            JobInfo* m_job = nullptr;

            bool is_job_ready_to_resume() const;            
        };
    }
}