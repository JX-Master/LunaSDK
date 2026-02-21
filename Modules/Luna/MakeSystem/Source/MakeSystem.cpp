/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MakeSystem.cpp
* @author JXMaster
* @date 2026/2/9
*/
#include "MakeSystem.hpp"
#include <Luna/Runtime/File.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/HashSet.hpp>
#include <Luna/Runtime/UniquePtr.hpp>

namespace Luna
{
    namespace MakeSystem
    {
        const c8 LOG_TAG[] = "MakeSystem";
        RV MakeSystem::init(const Path& build_dir, u32 max_num_parallel_tasks)
        {
            lutry
            {
                if(build_dir.empty())
                {
                    const c8* buf = get_current_dir();
                    m_build_dir = buf;
                    m_build_dir.push_back("build");
                    release_current_dir(buf);
                }
                else
                {
                    m_build_dir = build_dir;
                }
                Path cache_dir = m_build_dir;
                cache_dir.push_back("build_cache");
                luexp(m_build_cache.init(cache_dir));
                luset(m_job_scheduler, JobSystem::new_job_scheduler(max_num_parallel_tasks))
            }
            lucatchret;
            return ok;
        }

        static RV visit_node(MakeNode* node, HashSet<MakeNode*>& visited, HashSet<MakeNode*>& visiting, Vector<MakeNode*>& ordered);

        static R<Vector<MakeNode*>> collect_nodes(Span<MakeNode*> targets)
        {
            Vector<MakeNode*> ordered;
            HashSet<MakeNode*> visited;
            HashSet<MakeNode*> visiting; // for detecting circular dependencies.
            lutry
            {
                for (MakeNode* target : targets)
                {
                    luexp(visit_node(target, visited, visiting, ordered));
                }
            }
            lucatchret;
            return ordered;
        }

        static RV visit_node(MakeNode* node, HashSet<MakeNode*>& visited, HashSet<MakeNode*>& visiting, Vector<MakeNode*>& ordered)
        {
            if(visiting.contains(node))
            {
                return set_error(BasicError::bad_arguments(), "Circular dependency detected at node: %s", node->path.encode().c_str());
            }
            if(!visited.insert(node).second)
            {
                return ok;
            }
            visiting.insert(node);
            for(MakeNode* dep : node->dependencies)
            {
                auto r = visit_node(dep, visited, visiting, ordered);
                if(failed(r)) return r;
            }
            visiting.erase(node);
            ordered.push_back(node);
            return ok;
        }

        struct MakeNodeGlobalContext
        {
            SpinLock m_lock;
            LogHandler* m_log_handler;
            Error m_error;
            usize m_num_jobs;
            usize m_num_finished_jobs;
            bool m_cancelled = false;
        };

        struct MakeNodeContext
        {
            MakeNodeGlobalContext* m_env;
            Ref<IMakeCommand> m_command;
            String m_display_info;
            Vector<JobSystem::job_id_t> m_deps;

            bool is_cancelled()
            {
                LockGuard guard(m_env->m_lock);
                return m_env->m_cancelled;
            }
        };

        static void make_task_entry(JobSystem::IJobScheduler* scheduler, void* params)
        {
            MakeNodeContext* ctx = (MakeNodeContext*)params;
            if(ctx->is_cancelled()) return;
            // Wait for all deps.
            scheduler->wait_jobs(ctx->m_deps.cspan());
            if(ctx->is_cancelled()) return;
            usize finsihed_jobs = atom_inc_usize(&ctx->m_env->m_num_finished_jobs);
            log_info(*ctx->m_env->m_log_handler, LOG_TAG, "[%llu/%llu]%s", (u64)finsihed_jobs, (u64)ctx->m_env->m_num_jobs, ctx->m_display_info.c_str());
            auto r = ctx->m_command->execute(*ctx->m_env->m_log_handler);
            if(failed(r))
            {
                auto env = ctx->m_env;
                LockGuard guard(env->m_lock);
                if(!env->m_cancelled)
                {
                    env->m_cancelled = true;
                    if(r.errcode() == BasicError::error_object())
                    {
                        env->m_error = get_error();
                    }
                    else
                    {
                        env->m_error.code = r.errcode();
                        env->m_error.message = explain(r.errcode());
                    }
                }
            }
        }

        RV MakeSystem::make(Span<MakeNode*> targets)
        {
            lutry
            {
                if(targets.empty())
                {
                    log_info(m_log_handler, LOG_TAG, "Nothing to make.");
                    return ok;
                }
                lulet(ordered_nodes, collect_nodes(targets));
                if(ordered_nodes.empty())
                {
                    log_info(m_log_handler, LOG_TAG, "Nothing to make.");
                    return ok;
                }

                // Loads all node timestamps.
                HashMap<MakeNode*, i64> node_timestamps;
                for(MakeNode* node : ordered_nodes)
                {
                    i64 t = get_node_timestamp(node);
                    node_timestamps.insert_or_assign(node, t);
                }
                HashSet<MakeNode*> needs_build;

                // Collect all nodes that should be built.
                for(MakeNode* node : ordered_nodes)
                {
                    i64 current_timestamp = node_timestamps[node];
                    i64 max_dep_timestamp = 0;
                    bool dependency_needs_build = false;

                    for (MakeNode* dependency : node->dependencies)
                    {
                        // If the dependency needs rebuild.
                        if(needs_build.contains(dependency))
                        {
                            dependency_needs_build = true;
                        }
                        max_dep_timestamp = max(max_dep_timestamp, node_timestamps[dependency]);
                    }

                    bool missing_file = node->has_file && failed(get_file_attribute(node->path.encode().c_str()));
                    bool should_build = node->force_rebuild || missing_file || max_dep_timestamp > current_timestamp || dependency_needs_build;
                    if(should_build)
                    {
                        needs_build.insert(node);
                    }
                }

                usize num_commands_to_build = needs_build.size();
                if(num_commands_to_build == 0)
                {
                    log_info(m_log_handler, LOG_TAG, "Nothing to make.");
                    return ok;
                }

                // Dispatch jobs.
                Vector<MakeNode*> build_jobs;
                for(MakeNode* node : ordered_nodes)
                {
                    if(!needs_build.contains(node))
                    {
                        continue;
                    }
                    build_jobs.push_back(node);
                }
                MakeNodeGlobalContext env;
                env.m_log_handler = &m_log_handler;
                env.m_num_jobs = build_jobs.size();
                env.m_num_finished_jobs = 0;
                HashMap<MakeNode*, JobSystem::job_id_t> job_id_map;
                job_id_map.reserve(build_jobs.size());
                Vector<JobSystem::job_id_t> job_ids;
                job_ids.reserve(build_jobs.size());
                HashMap<MakeNode*, UniquePtr<MakeNodeContext>> job_ctxs;
                job_ctxs.reserve(build_jobs.size());
                for(MakeNode* node : build_jobs)
                {
                    UniquePtr<MakeNodeContext> ctx(memnew<MakeNodeContext>());
                    ctx->m_env = &env;
                    ctx->m_command = node->command;
                    ctx->m_display_info = node->display_info;
                    for(MakeNode* dep : node->dependencies)
                    {
                        auto iter = job_id_map.find(dep);
                        if(iter != job_id_map.end())
                        {
                            ctx->m_deps.push_back(iter->second);
                        }
                    }
                    auto id = m_job_scheduler->submit_job(make_task_entry, ctx.get());
                    job_ctxs.insert(make_pair(node, move(ctx)));
                    job_id_map.insert(make_pair(node, id));
                    job_ids.push_back(id);
                }
                // Wait for all jobs.
                m_job_scheduler->wait_jobs(job_ids.cspan());
                if(env.m_cancelled)
                {
                    get_error() = env.m_error;
                    return BasicError::error_object();
                }
            }
            lucatchret;
            return ok;
        }

        void MakeSystem::set_log_handler(const LogHandler& handler)
        {
            m_log_handler = handler.valid() ? handler : default_log_handler;
        }

        void default_log_handler(LogVerbosity verbosity, const c8* tag, usize tag_length, const c8* message, usize message_length)
        {
            log_unformatted(verbosity, tag, tag_length, message, message_length);
        }

        i64 MakeSystem::get_node_timestamp(MakeNode* node)
        {
            if(node->has_file)
            {
                auto r = get_file_attribute(node->path.encode().c_str());
                if(failed(r))
                {
                    return 0;
                }
                return r.get().last_write_time;
            }
            return m_build_cache.get_timestamp(node->path);
        }
    }
}