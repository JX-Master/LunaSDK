/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file JobScheduler.hpp
* @author JXMaster
* @date 2026/2/15
*/
#pragma once
#include "../JobScheduler.hpp"
#include "JobContext.hpp"
#include <Luna/Runtime/SpinLock.hpp>
#include <Luna/Runtime/RingDeque.hpp>
#include <Luna/Runtime/Signal.hpp>

namespace Luna
{
    namespace JobSystem
    {
        struct WorkerThreadContext
        {
            JobScheduler* m_owner = nullptr;
            SpinLock m_lock;
            RingDeque<JobInfo*> m_jobs; // jobs that is waiting for executing.
            Vector<JobContext*> m_waiting_jobs; // jobs that are blocked because they are waiting for other jobs.
            JobContext* m_current_job = nullptr; // The current executing job. Only valid when this is a worker thread.
            Ref<ISignal> m_wake_signal;
            Ref<IFiber> m_worker_thread_fiber;
            bool m_thread_dead = false; // Set to true when thread returns.
        };

        struct JobScheduler : IJobScheduler
        {
            lustruct("JobSystem::JobScheduler", "{f198bcbd-8cf1-4223-a777-1a2d6e9820d6}");
            luiimpl();

            RV init(u32 num_worker_threads);
            ~JobScheduler();

            // Begin of job contexts.
            SpinLock m_job_contexts_lock;
            Vector<JobContext*> m_job_contexts;
            Vector<JobContext*> m_free_job_contexts;
            static constexpr usize JOB_FIBER_STACK_SIZE = 256_kb;

            JobContext* allocate_job_context();
            void free_job_context(JobContext* ctx);
            // End of job contexts.

            // Begin of job state map.
            SpinLock m_job_state_map_lock;
            job_id_t m_next_job_id;
            RingDeque<u64> m_job_state_map;
            usize m_job_state_map_offset;
            static constexpr usize JOBS_PER_CHUNK = 64;

            job_id_t job_state_map_begin()
            {
                return m_job_state_map_offset * JOBS_PER_CHUNK;
            }
            job_id_t job_state_map_end()
            {
                return (m_job_state_map_offset + m_job_state_map.size()) * JOBS_PER_CHUNK;
            }
            virtual job_id_t allocate_job_id() override;
            virtual void finish_job_id(job_id_t id) override;
            virtual bool is_job_finished(job_id_t id) override;
            // End of job state map.

            // Begin of job worker thread state.
            SpinLock m_jobs_lock;
            RingDeque<JobInfo*> m_jobs; // Jobs submitted from user thread.
            SpinLock m_worker_thread_contexts_lock;
            Vector<WorkerThreadContext*> m_worker_thread_contexts;
            Vector<Ref<IThread>> m_worker_threads;
            SpinLock m_sleep_worker_threads_lock;
            Vector<WorkerThreadContext*> m_sleep_worker_threads;
            bool m_job_system_exiting = false;

            virtual u32 get_num_worker_threads() override
            {
                return (u32)m_worker_threads.size();
            }
            WorkerThreadContext* get_current_thread_worker_context();
            void wait_for_all_jobs();
            JobInfo* steal_job(WorkerThreadContext* current_ctx);
            JobInfo* consume_job(WorkerThreadContext* ctx);
            void finish_job(JobInfo* job);
            void worker_thread_sleep(WorkerThreadContext* ctx);
            void resume_job(WorkerThreadContext* ctx, JobContext* job);
            virtual job_id_t submit_job(void (*func)(IJobScheduler* scheduler, void* params), void* params) override;
            virtual void wait_job(job_id_t job) override;
            virtual void wait_jobs(Span<const job_id_t> jobs) override;
            // End of job worker thread state.
        };
    }
}