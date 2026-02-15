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
#include "JobScheduler.hpp"

namespace Luna
{
    namespace JobSystem
    {
        bool JobContext::is_job_ready_to_resume() const
        {
            for(job_id_t job : m_wait_jobs)
            {
                if(!m_owner->is_job_finished(job))
                {
                    return false;
                }
            }
            return true;
        }
    }
}