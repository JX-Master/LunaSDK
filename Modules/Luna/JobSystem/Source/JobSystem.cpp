/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file JobSystem.cpp
* @author JXMaster
* @date 2022/7/7
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_JOBSYSTEM_API LUNA_EXPORT
#include "JobStateMap.hpp"
#include "JobContext.hpp"
#include <Luna/Runtime/RingDeque.hpp>
#include <Luna/Runtime/SpinLock.hpp>
#include <Luna/Runtime/Signal.hpp>
#include <Luna/Runtime/Random.hpp>
#include <Luna/Runtime/Module.hpp>

namespace Luna
{
    namespace JobSystem
    {
        struct WorkerThreadContext
        {
            SpinLock m_lock;
            RingDeque<JobInfo*> m_jobs; // jobs that is waiting for executing.
            Vector<JobContext*> m_waiting_jobs; // jobs that are blocked because they are waiting for other jobs.
            JobContext* m_current_job = nullptr; // The current executing job. Only valid when this is a worker thread.
            Ref<ISignal> m_wake_signal;
            Ref<IFiber> m_worker_thread_fiber; // null if this thread is not a worker thread.
            bool m_thread_dead = false; // Set to true when thread returns.

            bool is_worker_thread() const
            {
                return m_worker_thread_fiber.valid();
            }
        };

        static SpinLock g_worker_thread_contexts_lock;
        static Vector<WorkerThreadContext*> g_worker_thread_contexts;
        static Vector<Ref<IThread>> g_worker_threads;
        static SpinLock g_sleep_worker_threads_lock;
        static Vector<WorkerThreadContext*> g_sleep_worker_threads;
        static opaque_t g_worker_thread_tls;
        static bool g_job_system_exiting;

        static void worker_thread_tls_dtor(void* params)
        {
            // Marks this context to be dead, so that it will be removed 
            // when the queue is clear, or when the system is shut down.
            WorkerThreadContext* ctx = (WorkerThreadContext*)params;
            ctx->m_thread_dead = true;
        }
        static void worker_thread_run(void* params);
        RV job_system_init()
        {
            init_job_state_map();
            g_job_system_exiting = false;
            g_worker_thread_tls = tls_alloc(worker_thread_tls_dtor);
            // Emit worker threads.
            u32 processor_count = get_processors_count();
            for (u32 i = 0; i < processor_count; ++i)
            {
                Ref<IThread> worker = new_thread(worker_thread_run, nullptr);
                g_worker_threads.push_back(worker);
            }
            // Consume job id 0, so any valid job will never have ID 0.
            job_id_t dummy = allocate_job_id();
            finish_job_id(dummy);
            return ok;
        }
        static void wait_for_all_jobs()
        {
            while(true)
            {
                {
                    LockGuard guard(g_sleep_worker_threads_lock);
                    if(g_sleep_worker_threads.size() == g_worker_threads.size())
                    {
                        // All worker threads are finished working.
                        break;
                    }
                }
                yield_current_thread();
            }
        }
        void job_system_close()
        {
            wait_for_all_jobs();
            g_job_system_exiting = true;
            // Wake up all sleep threads.
            for (auto& t : g_sleep_worker_threads)
            {
                t->m_wake_signal->trigger();
            }
            // Wait for all threads to exit.
            g_worker_threads.clear();
            g_worker_threads.shrink_to_fit();
            // Clean up contexts.
            tls_free(g_worker_thread_tls);
            g_worker_thread_contexts_lock.lock();
            for (WorkerThreadContext* ctx : g_worker_thread_contexts)
            {
                memdelete(ctx);
            }
            g_worker_thread_contexts.clear();
            g_worker_thread_contexts.shrink_to_fit();
            g_worker_thread_contexts_lock.unlock();
            g_sleep_worker_threads.clear();
            g_sleep_worker_threads.shrink_to_fit();
            clean_up_job_contexts();
            close_job_state_map();
        }
        static WorkerThreadContext* get_current_thread_worker_context()
        {
            WorkerThreadContext* ctx = (WorkerThreadContext*)tls_get(g_worker_thread_tls);
            if (!ctx)
            {
                // We create context here so that user-created threads can also have a 
                // worker context.
                ctx = memnew<WorkerThreadContext>();
                tls_set(g_worker_thread_tls, ctx);
                g_worker_thread_contexts_lock.lock();
                g_worker_thread_contexts.push_back(ctx);
                g_worker_thread_contexts_lock.unlock();
            }
            return ctx;
        }
        inline JobInfo* steal_job(WorkerThreadContext* current_ctx)
        {
            LockGuard guard(g_worker_thread_contexts_lock);
            if (g_worker_thread_contexts.empty()) return nullptr;
            u32 rand_index = random_u32() % (u32)g_worker_thread_contexts.size();
            usize i = 0;
            // Iterate all thread contexts, starting from index i.
            while (i < g_worker_thread_contexts.size())
            {
                u32 index = (rand_index + i) % (u32)g_worker_thread_contexts.size();
                WorkerThreadContext* steal_ctx = g_worker_thread_contexts[index];
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
                    g_worker_thread_contexts.erase(g_worker_thread_contexts.begin() + index);
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
            return nullptr;
        }
        static JobInfo* consume_job(WorkerThreadContext* ctx)
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
        inline void finish_job(JobInfo* job)
        {
            job->m_finished = true;
            finish_job_id(job->m_id);
        }

        void job_entry(void* params)
        {
            JobContext* job = (JobContext*)params;
            while(!g_job_system_exiting)
            {
                // Process the current job.
                job->m_job->m_func(job->m_job->m_params);
                finish_job(job->m_job);
                // One job is finished, give control back to root coroutine.
                yield_coroutine();
            }
        }

        static void worker_thread_sleep(WorkerThreadContext* ctx)
        {
            if (!ctx->m_wake_signal) ctx->m_wake_signal = new_signal(false);
            // Add the current thread to sleep list.
            g_sleep_worker_threads_lock.lock();
            g_sleep_worker_threads.push_back(ctx);
            g_sleep_worker_threads_lock.unlock();
            ctx->m_wake_signal->wait();
        }
        static void resume_job(WorkerThreadContext* ctx, JobContext* job)
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
        static void worker_thread_run(void* params)
        {
            WorkerThreadContext* ctx = get_current_thread_worker_context();
            auto fiber = convert_thread_to_fiber();
            if(failed(fiber))
            {
                lupanic_msg_always("failed to create fiber context for worker thread.");
            }
            ctx->m_worker_thread_fiber = fiber.get();
            // Root coroutine.
            while (!g_job_system_exiting)
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
                            resume_job(ctx, job);
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
                    JobInfo* job = consume_job(ctx);
                    if (job)
                    {
                        JobContext* job_ctx = allocate_job_context();
                        job_ctx->m_job = job;
                        resume_job(ctx, job_ctx);
                    }
                    else if(ctx->m_waiting_jobs.empty())
                    {
                        // We have processed all jobs, put this thread to sleep.
                        worker_thread_sleep(ctx);
                    }
                }
            }
            ctx->m_worker_thread_fiber.reset();
            lupanic_if_failed(convert_fiber_to_thread());
        }
        LUNA_JOBSYSTEM_API job_id_t submit_job(void (*func)(void* params), void* params)
        {
            JobInfo* job = memnew<JobInfo>();
            job_id_t id = allocate_job_id();
            job->m_id = id;
            job->m_func = func;
            job->m_params = params;
            job->m_finished = false;
            // Push the job to the job queue of the current thread.
            WorkerThreadContext* ctx = get_current_thread_worker_context();
            LockGuard lock(ctx->m_lock);
            ctx->m_jobs.push_back(job);
            // Wake up one worker thread if any.
            g_sleep_worker_threads_lock.lock();
            if (!g_sleep_worker_threads.empty())
            {
                WorkerThreadContext* worker = g_sleep_worker_threads.back();
                g_sleep_worker_threads.pop_back();
                worker->m_wake_signal->trigger();
            }
            g_sleep_worker_threads_lock.unlock();
            return id;
        }
        LUNA_JOBSYSTEM_API void wait_job(job_id_t job)
        {
            WorkerThreadContext* ctx = get_current_thread_worker_context();
            if(!ctx->is_worker_thread())
            {
                // Spin-waiting loop.
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
        LUNA_JOBSYSTEM_API void wait_jobs(Span<const job_id_t> jobs)
        {
            WorkerThreadContext* ctx = get_current_thread_worker_context();
            if(!ctx->is_worker_thread())
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

        struct JobSystemModule : public Module
        {
            virtual const c8* get_name() override { return "JobSystem"; }
            virtual RV on_init() override
            {
                return job_system_init();
            }
            virtual void on_close() override
            {
                job_system_close();
            }
        };
    }
    LUNA_JOBSYSTEM_API Module* module_job_system()
    {
        static JobSystem::JobSystemModule m;
        return &m;
    }
}
