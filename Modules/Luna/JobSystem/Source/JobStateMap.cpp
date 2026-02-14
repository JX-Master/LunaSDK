/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file JobStateMap.cpp
* @author JXMaster
* @date 2026/2/14
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_JOBSYSTEM_API LUNA_EXPORT
#include "JobStateMap.hpp"

#include <Luna/Runtime/SpinLock.hpp>
#include <Luna/Runtime/RingDeque.hpp>

namespace Luna
{
    namespace JobSystem
    {
        // Used to record job states even when the job context is destroyed.
        static SpinLock g_job_state_map_lock;
        static job_id_t g_next_job_id;
        static RingDeque<u64> g_job_state_map;
        static usize g_job_state_map_offset;
        constexpr usize JOBS_PER_CHUNK = 64;
        void init_job_state_map()
        {
            g_next_job_id = 0;
            g_job_state_map_offset = 0;
        }
        void close_job_state_map()
        {
            g_job_state_map.clear();
            g_job_state_map.shrink_to_fit();
        }
        inline job_id_t job_state_map_begin()
        {
            return g_job_state_map_offset * JOBS_PER_CHUNK;
        }
        inline job_id_t job_state_map_end()
        {
            return (g_job_state_map_offset + g_job_state_map.size()) * JOBS_PER_CHUNK;
        }
        LUNA_JOBSYSTEM_API job_id_t allocate_job_id()
        {
            LockGuard guard(g_job_state_map_lock);
            job_id_t r = g_next_job_id;
            ++g_next_job_id;
            if (r >= job_state_map_end())
            {
                g_job_state_map.push_back(0);
            }
            return r;
        }
        LUNA_JOBSYSTEM_API void finish_job_id(job_id_t id)
        {
            LockGuard guard(g_job_state_map_lock);
            luassert(id >= job_state_map_begin() && id < job_state_map_end());
            usize chunk_index = id / JOBS_PER_CHUNK - g_job_state_map_offset;
            usize bit_offset = id % JOBS_PER_CHUNK;
            u64* chunk = &(g_job_state_map[chunk_index]);
            bit_set(chunk, bit_offset);
            if (chunk_index == 0)
            {
                while (*chunk == U64_MAX)
                {
                    ++g_job_state_map_offset;
                    g_job_state_map.pop_front();
                    if (g_job_state_map.empty()) break;
                    chunk = &g_job_state_map.front();
                }
            }
            guard.unlock();
        }
        LUNA_JOBSYSTEM_API bool is_job_finished(job_id_t id)
        {
            LockGuard guard(g_job_state_map_lock);
            if (id < job_state_map_begin()) return true;
            if (id >= job_state_map_end()) return false;
            usize chunk_index = id / JOBS_PER_CHUNK - g_job_state_map_offset;
            usize bit_offset = id % JOBS_PER_CHUNK;
            u64* chunk = &(g_job_state_map[chunk_index]);
            return bit_test(chunk, bit_offset);
        }
    }
}