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
#include "JobSystem.hpp"
#include "JobContext.hpp"
#include "JobSchedulerImpl.hpp"
#include "JobSystem.meta.generated.hpp"
#include <Luna/Runtime/RingDeque.hpp>
#include <Luna/Runtime/SpinLock.hpp>
#include <Luna/Runtime/Random.hpp>
#include <Luna/Runtime/Module.hpp>

namespace Luna
{
    namespace JobSystem
    {
        opaque_t g_worker_thread_tls;

        RV job_system_init()
        {
            Meta::register_JobSystem_types();
            lutry
            {
                luset(g_worker_thread_tls, tls_alloc());
            }
            lucatchret;
            return ok;
        }
        void job_system_close()
        {
            tls_free(g_worker_thread_tls);
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
