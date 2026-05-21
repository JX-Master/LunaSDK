/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MakeSystem.hpp
* @author JXMaster
* @date 2026/2/9
*/
#pragma once
#include "../MakeSystem.hpp"
#include "BuildCache.hpp"
#include <Luna/JobSystem/JobScheduler.hpp>
#include "MakeSystem.generated.hpp"
namespace Luna
{
    namespace MakeSystem
    {
        void default_log_handler(LogVerbosity verbosity, const c8* tag, usize tag_length, const c8* message, usize message_length);
        struct [[luna::struct("{3374ca8c-5fe8-4d6e-ad19-07c42a7da1fc}")]] MakeSystem : IMakeSystem
        {
            luiimpl();

            Path m_build_dir;
            BuildCache m_build_cache;
            Ref<JobSystem::IJobScheduler> m_job_scheduler;
            LogHandler m_log_handler = default_log_handler;

            RV init(const Path& build_dir, u32 max_num_parallel_tasks);

            virtual const Path& get_build_dir() override
            {
                return m_build_dir;
            }

            virtual i32 get_max_num_parallel_tasks() override
            {
                return m_job_scheduler->get_num_worker_threads();
            }

            virtual RV make(Span<MakeNode*> targets) override;

            virtual void set_log_handler(const LogHandler& handler) override;

            i64 get_node_timestamp(MakeNode* node);
        };
    }
}
