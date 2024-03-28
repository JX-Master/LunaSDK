/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file JobSystem.hpp
* @author JXMaster
* @date 2022/7/7
*/
#pragma once
#include <Luna/Runtime/Base.hpp>
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

        //! The callback function of one job.
        //! @param[in] params The parameter passed to @ref submit_job.
        using job_func_t = void(void* params);

        //! Allocates one job ID, so that other threads can wait for it by calling @ref wait_job.
        //! @return Returns the allocated job ID.
        //! @remark This function is called internally by the job system for all jobs submitted by @ref submit_job, so the user doesn't need to call this function manually.
        //! However, the job ID can also be used solely without submitting any job to provide a synchronizing point that other threads can wait for.
        //! 
        //! Every allocated job ID must be finished by calling @ref finish_job_id, or memory leak will occur. For job IDs created by @ref submit_job, the job system calls @ref finish_job_id 
        //! automatically when the job callback function returns, so the user should not finish it manually. But for job IDs created by @ref allocate_job_id, the user should call
        //! @ref finish_job_id manually to correctly finish them.
        LUNA_JOBSYSTEM_API job_id_t allocate_job_id();

        //! Marks one job ID as finished, so that all jobs waiting for this job ID will be resumed.
        //! This function should only be called for job IDs allocated by @ref allocate_job_id, never call this function for job IDs returned by @ref submit_job.
        //! See remarks of @ref allocate_job_id for details.
        LUNA_JOBSYSTEM_API void finish_job_id(job_id_t job);

        //! Creates a new job.
        //! @param[in] func The job callback function to invoke.
        //! @param[in] param_size The size of the parameter block.
        //! @param[in] param_alignment The alignment of the parameter block.
        //! @param[in] parent The optional parameter pointer of the parent job. If this is not `nullptr`, all waits for the parent
        //! job will wait this job as well.
        //! @return Returns the parameter block pointer of the created job. The parameter block data is uninitialized and should be 
        //! initialized by the user.
        LUNA_JOBSYSTEM_API void* new_job(job_func_t* func, usize param_size, usize param_alignment, void* parent = nullptr);

        //! Submits the job to the job system.
        //! @param[in] params The parameter block pointer of the job. Every job can only be submitted once.
        //! If the parameter block is not trivially destructable, the user must destruct the parameter block manually at the end of the
        //! job callback function.
        //! @return Returns the job ID for the submitted job, which can be used to wait for the job using @ref wait_job, or check whether
        //! the job is finished using @ref is_job_finished.
        LUNA_JOBSYSTEM_API job_id_t submit_job(void* params);

        //! Fetches the job ID assigned with the specified job.
        //! @param[in] params The parameter block pointer of the job.
        //! @return Returns the assigned job ID for the job.
        //! Returns @ref INVALID_JOB_ID if the job is not submitted yet.
        LUNA_JOBSYSTEM_API job_id_t get_current_job_id(void* params);

        //! Blocks the current thread to wait for the job to finish.
        //! @param[in] job The job ID to wait. If this is @ref INVALID_JOB_ID, this call returns immediately.
        LUNA_JOBSYSTEM_API void wait_job(job_id_t job);

        //! Checks whether the specified job is finished.
        //! @param[in] job The job ID to check. If this is @ref INVALID_JOB_ID, this call always return `true`.
        //! @return Returns `true` if the job is finished, `false` otherwise.
        LUNA_JOBSYSTEM_API bool is_job_finished(job_id_t job);

        //! @}
    }

    struct Module;
    LUNA_JOBSYSTEM_API Module* module_job_system();
}