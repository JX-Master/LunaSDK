/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file JobScheduler.hpp
* @author JXMaster
* @date 2025/2/15
*/
#pragma once
#include <Luna/Runtime/Span.hpp>
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/Runtime/Result.hpp>

#ifndef LUNA_JOBSYSTEM_API
#define LUNA_JOBSYSTEM_API
#endif

namespace Luna
{
    namespace JobSystem
    {
        //! @addtogroup JobSystem Job System
        //! Job system module provides thread pool to execute jobs asynchronously for a multithreaded computer program.
        //! @{
        
        //! Identifies one job that can be waited.
        using job_id_t = u64;

        //! A special ID that identifies one invalid job.
        constexpr job_id_t INVALID_JOB_ID = 0;

        //! @interface IJobScheduler
        //! Represents one job scheduler that owns several worker threads and can dispatch jobs to these 
        //! worker threads.
        struct IJobScheduler : virtual Interface
        {
            luiid("{15dfb63a-a3a9-413a-ad6c-d0db846d2e35}");

            //! Gets the number of worker threads in this scheduler.
            //! @return Returns the number of worker threads.
            virtual u32 get_num_worker_threads() = 0;

            //! Allocates one job ID, so that other threads can wait for it by calling @ref wait_job.
            //! @return Returns the allocated job ID.
            //! @remark This function is called internally by the job system for all jobs submitted by @ref submit_job, so the user doesn't need to call this function manually.
            //! However, the job ID can also be used solely without submitting any job to provide a synchronizing point that other threads can wait for.
            //! 
            //! Every allocated job ID must be finished by calling @ref finish_job_id, or memory leak will occur. For job IDs created by @ref submit_job, the job system calls @ref finish_job_id 
            //! automatically when the job callback function returns, so the user should not finish it manually. But for job IDs created by @ref allocate_job_id, the user should call
            //! @ref finish_job_id manually to correctly finish them.
            virtual job_id_t allocate_job_id() = 0;

            //! Marks one job ID as finished, so that all jobs waiting for this job ID will be resumed.
            //! This function should only be called for job IDs allocated by @ref allocate_job_id, never call this function for job IDs returned by @ref submit_job.
            //! See remarks of @ref allocate_job_id for details.
            virtual void finish_job_id(job_id_t job) = 0;

            //! Checks whether the specified job is finished.
            //! @param[in] job The job ID to check. If this is @ref INVALID_JOB_ID, this call always return `true`.
            //! @return Returns `true` if the job is finished, `false` otherwise.
            virtual bool is_job_finished(job_id_t job) = 0;

            //! Submits one job to the job system.
            //! @param[in] func The job function to invoke.
            //! @param[in] params The opaque pointer that will be passed to job function as parameter.
            //! @return Returns the job ID for the submitted job, which can be used to wait for the job using @ref wait_job, or check whether
            //! the job is finished using @ref is_job_finished.
            virtual job_id_t submit_job(void (*func)(IJobScheduler* scheduler, void* params), void* params) = 0;

            //! Blocks the current thread to wait for the job to finish.
            //! @param[in] job The job ID to wait. If this is @ref INVALID_JOB_ID, this call returns immediately.
            virtual void wait_job(job_id_t job) = 0;

            //! Blocks the current thread to wait for all jobs to finish.
            //! @details Call this function instead of repeatly call @ref wait_job if you need to wait for 
            //! many jobs, since every @ref wait_job call will trigger a job schedule operation, but this function
            //! only use one job schedule operation to submit all job dependencies, thus improves performance.
            //! @param[in] jobs A span of jobs to wait.
            virtual void wait_jobs(Span<const job_id_t> jobs) = 0;
        };

        //! Creates a new job scheduler.
        //! @param[in] num_worker_threads The number of worker threads in this job scheduler.
        //! If this is `0`, the number of worker threads is the same as the number of processor cores 
        //! of the platform.
        //! @return Returns the created job scheduler.
        LUNA_JOBSYSTEM_API R<Ref<IJobScheduler>> new_job_scheduler(u32 num_worker_threads = 0);

        //! @}
    }
}