/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file JobScheduler.cpp
* @author JXMaster
* @date 2026/2/15
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_JOBSYSTEM_API LUNA_EXPORT
#include "JobScheduler.hpp"
#include "JobSystem.hpp"
#include <Luna/Runtime/Random.hpp>

namespace Luna
{
    namespace JobSystem
    {
        static void job_entry(void* params);
        static void worker_thread_entry(void* params);

        RV JobScheduler::init(u32 num_worker_threads)
        {
            m_next_job_id = 0;
            m_job_state_map_offset = 0;
            m_job_system_exiting = false;
            // Emit worker threads.
            num_worker_threads = num_worker_threads == 0 ? get_processors_count() : num_worker_threads;
            for (u32 i = 0; i < num_worker_threads; ++i)
            {
                Ref<IThread> worker = new_thread(worker_thread_entry, this);
                m_worker_threads.push_back(worker);
            }   
            // Consume job id 0, so any valid job will never have ID 0.
            job_id_t dummy = allocate_job_id();
            finish_job_id(dummy);
            return ok;
        }
        JobScheduler::~JobScheduler()
        {
            wait_for_all_jobs();
            m_job_system_exiting = true;
            // Wake up all sleep threads.
            for (auto& t : m_sleep_worker_threads)
            {
                t->m_wake_signal->trigger();
            }
            m_sleep_worker_threads.clear();
            // Wait for all threads to exit.
            m_worker_threads.clear();
            // Clean up worker thread contexts.
            for (WorkerThreadContext* ctx : m_worker_thread_contexts)
            {
                memdelete(ctx);
            }
            m_worker_thread_contexts.clear();
            // Clean up job contexts.
            for(JobContext* job : m_job_contexts)
            {
                memdelete(job);
            }
            m_job_contexts.clear();
            m_free_job_contexts.clear();
            m_job_state_map.clear();
        }
        JobContext* JobScheduler::allocate_job_context()
        {
            LockGuard guard(m_job_contexts_lock);
            if(m_free_job_contexts.empty())
            {
                // Create new job context.
                JobContext* ctx = memnew<JobContext>();
                ctx->m_owner = this;
                auto coroutine = new_coroutine(JOB_FIBER_STACK_SIZE, job_entry, ctx);
                if (failed(coroutine)) [[unlikely]]
                {
                    lupanic_msg_always("Failed to create coroutine context for job");
                }
                ctx->m_coroutine = coroutine.get();
                m_job_contexts.push_back(ctx);
                return ctx;
            }
            JobContext* ctx = m_free_job_contexts.back();
            m_free_job_contexts.pop_back();
            return ctx;
        }
        void JobScheduler::free_job_context(JobContext* ctx)
        {
            LockGuard guard(m_job_contexts_lock);
            m_free_job_contexts.push_back(ctx);
        }
        job_id_t JobScheduler::allocate_job_id()
        {
            LockGuard guard(m_job_state_map_lock);
            job_id_t r = m_next_job_id;
            ++m_next_job_id;
            if (r >= job_state_map_end())
            {
                m_job_state_map.push_back(0);
            }
            return r;
        }
        void JobScheduler::finish_job_id(job_id_t id)
        {
            LockGuard guard(m_job_state_map_lock);
            luassert(id >= job_state_map_begin() && id < job_state_map_end());
            usize chunk_index = id / JOBS_PER_CHUNK - m_job_state_map_offset;
            usize bit_offset = id % JOBS_PER_CHUNK;
            u64* chunk = &(m_job_state_map[chunk_index]);
            bit_set(chunk, bit_offset);
            if (chunk_index == 0)
            {
                while (*chunk == U64_MAX)
                {
                    ++m_job_state_map_offset;
                    m_job_state_map.pop_front();
                    if (m_job_state_map.empty()) break;
                    chunk = &m_job_state_map.front();
                }
            }
            guard.unlock();
        }
        bool JobScheduler::is_job_finished(job_id_t id)
        {
            LockGuard guard(m_job_state_map_lock);
            if (id < job_state_map_begin()) return true;
            if (id >= job_state_map_end()) return false;
            usize chunk_index = id / JOBS_PER_CHUNK - m_job_state_map_offset;
            usize bit_offset = id % JOBS_PER_CHUNK;
            u64* chunk = &(m_job_state_map[chunk_index]);
            return bit_test(chunk, bit_offset);
        }
        WorkerThreadContext* JobScheduler::get_current_thread_worker_context()
        {
            WorkerThreadContext* ctx = (WorkerThreadContext*)tls_get(g_worker_thread_tls);
            if(ctx && ctx->m_owner == this)
            {
                return ctx;
            }
            return nullptr;
        }
        void JobScheduler::wait_for_all_jobs()
        {
            while(true)
            {
                {
                    LockGuard guard(m_sleep_worker_threads_lock);
                    if(m_sleep_worker_threads.size() == m_worker_threads.size())
                    {
                        // All worker threads are finished working.
                        break;
                    }
                }
                yield_current_thread();
            }
        }
        JobInfo* JobScheduler::steal_job(WorkerThreadContext* current_ctx)
        {
            LockGuard guard(m_worker_thread_contexts_lock);
            u32 rand_range = m_worker_thread_contexts.size() + 1;
            u32 rand_index = random_u32() % (u32)rand_range;
            usize i = 0;
            // Iterate all thread contexts, starting from index i.
            while (i < rand_range)
            {
                u32 index = (rand_index + i) % rand_range;
                if(index == m_worker_thread_contexts.size())
                {
                    // Steal from global job queue.
                    m_jobs_lock.lock();
                    JobInfo* job = nullptr;
                    if(!m_jobs.empty())
                    {
                        job = m_jobs.front();
                        m_jobs.pop_front();
                    }
                    m_jobs_lock.unlock();
                    if (job) return job;
                    ++i;
                }
                else
                {
                    WorkerThreadContext* steal_ctx = m_worker_thread_contexts[index];
                    if (steal_ctx == current_ctx)
                    {
                        ++i;
                        continue;
                    }
                    steal_ctx->m_lock.lock();
                    if (steal_ctx->m_thread_dead && steal_ctx->m_jobs.empty())
                    {
                        // Remove this context.
                        steal_ctx->m_lock.unlock();
                        m_worker_thread_contexts.erase(m_worker_thread_contexts.begin() + index);
                        memdelete(steal_ctx);
                    }
                    else
                    {
                        JobInfo* job = nullptr;
                        if (!steal_ctx->m_jobs.empty())
                        {
                            job = steal_ctx->m_jobs.front();
                            steal_ctx->m_jobs.pop_front();
                        }
                        steal_ctx->m_lock.unlock();
                        if (job) return job;
                        ++i;
                    }
                }
            }
            return nullptr;
        }
        JobInfo* JobScheduler::consume_job(WorkerThreadContext* ctx)
        {
            ctx->m_lock.lock();
            if (ctx->m_jobs.empty())
            {
                ctx->m_lock.unlock();
                // Steal jobs from other threads.
                JobInfo* job = steal_job(ctx);
                if (!job)
                {
                    yield_current_thread();
                    return nullptr;
                }
                return job;
            }
            else
            {
                JobInfo* job = ctx->m_jobs.back();
                ctx->m_jobs.pop_back();
                ctx->m_lock.unlock();
                return job;
            }
        }
        void JobScheduler::finish_job(JobInfo* job)
        {
            job->m_finished = true;
            finish_job_id(job->m_id);
        }
        void job_entry(void* params)
        {
            JobContext* job = (JobContext*)params;
            while(!job->m_owner->m_job_system_exiting)
            {
                // Process the current job.
                job->m_job->m_func(job->m_owner, job->m_job->m_params);
                job->m_owner->finish_job(job->m_job);
                // One job is finished, give control back to root coroutine.
                yield_coroutine();
            }
        }
        void JobScheduler::worker_thread_sleep(WorkerThreadContext* ctx)
        {
            if (!ctx->m_wake_signal) ctx->m_wake_signal = new_signal(false);
            // Add the current thread to sleep list.
            m_sleep_worker_threads_lock.lock();
            m_sleep_worker_threads.push_back(ctx);
            m_sleep_worker_threads_lock.unlock();
            ctx->m_wake_signal->wait();
        }
        void JobScheduler::resume_job(WorkerThreadContext* ctx, JobContext* job)
        {
            // Set up job context.
            ctx->m_current_job = job;
            // Resume job.
            resume_coroutine(job->m_coroutine);
            ctx->m_current_job = nullptr;
            if(job->m_job->m_finished)
            {
                memdelete(job->m_job);
                job->m_job = nullptr;
                free_job_context(job);
            }
        }
        static void worker_thread_entry(void* params)
        {
            WorkerThreadContext* ctx = memnew<WorkerThreadContext>();
            JobScheduler* owner = (JobScheduler*)params;
            ctx->m_owner = owner;
            tls_set(g_worker_thread_tls, ctx);
            owner->m_worker_thread_contexts_lock.lock();
            owner->m_worker_thread_contexts.push_back(ctx);
            owner->m_worker_thread_contexts_lock.unlock();
            auto fiber = convert_thread_to_fiber();
            if(failed(fiber))
            {
                lupanic_msg_always("failed to create fiber context for worker thread.");
            }
            ctx->m_worker_thread_fiber = fiber.get();
            // Root coroutine.
            while (!owner->m_job_system_exiting)
            {
                // Checks for all waiting jobs.
                bool any_waiting_job_resumed = false;
                {
                    auto iter = ctx->m_waiting_jobs.begin();
                    while(iter != ctx->m_waiting_jobs.end())
                    {
                        JobContext* job = *iter;
                        if(job->is_job_ready_to_resume())
                        {
                            iter = ctx->m_waiting_jobs.swap_erase(iter);
                            owner->resume_job(ctx, job);
                            any_waiting_job_resumed = true;
                        }
                        else
                        {
                            ++iter;
                        }
                    }
                }
                if(!any_waiting_job_resumed)
                {
                    // If no job needs to be resumed, we can process new jobs.
                    JobInfo* job = owner->consume_job(ctx);
                    if (job)
                    {
                        JobContext* job_ctx = owner->allocate_job_context();
                        job_ctx->m_job = job;
                        owner->resume_job(ctx, job_ctx);
                    }
                    else if(ctx->m_waiting_jobs.empty())
                    {
                        // We have processed all jobs, put this thread to sleep.
                        owner->worker_thread_sleep(ctx);
                    }
                }
            }
            ctx->m_worker_thread_fiber.reset();
            lupanic_if_failed(convert_fiber_to_thread());
        }
        job_id_t JobScheduler::submit_job(void (*func)(IJobScheduler* scheduler, void* params), void* params)
        {
            JobInfo* job = memnew<JobInfo>();
            job_id_t id = allocate_job_id();
            job->m_id = id;
            job->m_func = func;
            job->m_params = params;
            job->m_finished = false;
            // Push the job to the job queue of the current thread.
            WorkerThreadContext* ctx = get_current_thread_worker_context();
            if(ctx)
            {
                // Submitted by worker thread.
                LockGuard lock(ctx->m_lock);
                ctx->m_jobs.push_back(job);
            }
            else
            {
                // Submitted by non-worker thread.
                LockGuard lock(m_jobs_lock);
                m_jobs.push_back(job);
            }
            // Wake up one worker thread if any.
            m_sleep_worker_threads_lock.lock();
            if (!m_sleep_worker_threads.empty())
            {
                WorkerThreadContext* worker = m_sleep_worker_threads.back();
                m_sleep_worker_threads.pop_back();
                worker->m_wake_signal->trigger();
            }
            m_sleep_worker_threads_lock.unlock();
            return id;
        }
        void JobScheduler::wait_job(job_id_t job)
        {
            WorkerThreadContext* ctx = get_current_thread_worker_context();
            if(!ctx)
            {
                // Non-worker thread will not execute jobs, they just busy-waiting for jobs.
                // This is a design purpose, so that important threads (like main thread) will
                // not be accidentally blocked by long-term jobs. Generally you should minimize
                // waiting from main thread since it breaks parallelism and may introduce significant 
                // delay.
                while(!is_job_finished(job))
                {
                    processor_pause();
                }
            }
            else
            {
                if(is_job_finished(job)) return;
                ctx->m_current_job->m_wait_jobs.clear();
                ctx->m_current_job->m_wait_jobs.push_back(job);
                ctx->m_waiting_jobs.push_back(ctx->m_current_job);
                yield_coroutine(); // Give control back to the root coroutine.
                // When this coroutine is resumed, all waiting jobs should be completed, so just return here.
            }
        }
        void JobScheduler::wait_jobs(Span<const job_id_t> jobs)
        {
            WorkerThreadContext* ctx = get_current_thread_worker_context();
            if(!ctx)
            {
                // Non-worker thread will not execute jobs, they just busy-waiting for jobs.
                // This is a design purpose, so that important threads (like main thread) will
                // not be accidentally blocked by long-term jobs. Generally you should minimize
                // waiting from main thread since it breaks parallelism and may introduce significant 
                // delay.
                bool finished;
                do
                {
                    finished = true;
                    for(auto& job : jobs)
                    {
                        if(!is_job_finished(job))
                        {
                            finished = false;
                            break;
                        }
                    }
                    if(!finished)
                    {
                        processor_pause();
                    }
                } while (!finished);
            }
            else
            {
                ctx->m_current_job->m_wait_jobs.clear();
                for(job_id_t job : jobs)
                {
                    if(!is_job_finished(job))
                    {
                        ctx->m_current_job->m_wait_jobs.push_back(job);
                    }
                }
                if(ctx->m_current_job->m_wait_jobs.empty())
                {
                    return;
                }
                ctx->m_waiting_jobs.push_back(ctx->m_current_job);
                yield_coroutine(); // Give control back to the root coroutine.
                // When this coroutine is resumed, all waiting jobs should be completed, so just return here.
            }
        }
        LUNA_JOBSYSTEM_API R<Ref<IJobScheduler>> new_job_scheduler(u32 num_worker_threads)
        {
            Ref<IJobScheduler> ret;
            lutry
            {
                auto o = new_object<JobScheduler>();
                luexp(o->init(num_worker_threads));
                ret = o;
            }
            lucatchret;
            return ret;
        }
    }
}