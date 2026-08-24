/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MakeSystem.cpp
* @author JXMaster
* @date 2026/2/9
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_MAKE_SYSTEM_API LUNA_EXPORT
#include "MakeSystemImpl.hpp"
#include "MakeSystem.meta.generated.hpp"
#include <Luna/Runtime/File.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/Hash.hpp>
#include <Luna/Runtime/HashSet.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Time.hpp>
#include <Luna/Runtime/UniquePtr.hpp>
#include <Luna/JobSystem/JobSystem.hpp>
#include <cstring>

namespace Luna
{
    namespace MakeSystem
    {
        const c8 LOG_TAG[] = "MakeSystem";

        static bool node_has_real_file(MakeNode* node)
        {
            return node->kind == MakeNodeKind::file;
        }

        static Vector<Path> get_node_outputs(MakeNode* node)
        {
            Vector<Path> outputs;
            if(node_has_real_file(node))
            {
                outputs.push_back(node->path);
            }
            for(MakeNode* output : node->outputs)
            {
                if(node_has_real_file(output))
                {
                    outputs.push_back(output->path);
                }
            }
            for(MakeNode* depfile : node->depfiles)
            {
                if(node_has_real_file(depfile))
                {
                    outputs.push_back(depfile->path);
                }
            }
            return outputs;
        }

        static i64 get_file_timestamp(const Path& path)
        {
            auto attr = get_file_attribute(path.encode().c_str());
            if(failed(attr))
            {
                return 0;
            }
            return attr.get().last_write_time;
        }

        static bool file_exists(const Path& path)
        {
            return succeeded(get_file_attribute(path.encode().c_str()));
        }

        static void hash_bytes(u64& h, const void* data, usize size)
        {
            h = memhash64(data, size, h);
        }

        static void hash_string(u64& h, const String& value)
        {
            hash_bytes(h, value.data(), value.size());
            c8 zero = 0;
            hash_bytes(h, &zero, sizeof(zero));
        }

        static void hash_path(u64& h, const Path& path)
        {
            String encoded = path.encode();
            hash_string(h, encoded);
        }

        static void hash_file_fingerprint(u64& h, const Path& path)
        {
            hash_path(h, path);
            auto attr = get_file_attribute(path.encode().c_str());
            if(succeeded(attr))
            {
                u64 size = attr.get().size;
                i64 write_time = attr.get().last_write_time;
                hash_bytes(h, &size, sizeof(size));
                hash_bytes(h, &write_time, sizeof(write_time));
            }
            else
            {
                u64 missing = U64_MAX;
                hash_bytes(h, &missing, sizeof(missing));
            }
        }

        static u64 compute_action_key(MakeNode* node)
        {
            u64 h = 14695981039346656037ull;
            hash_path(h, node->path);
            hash_string(h, node->action.command);
            for(const auto& output : get_node_outputs(node))
            {
                hash_path(h, output);
            }
            return h;
        }

        static i64 get_outputs_timestamp(MakeNode* node)
        {
            Vector<Path> outputs = get_node_outputs(node);
            if(outputs.empty())
            {
                return 0;
            }
            i64 ret = I64_MAX;
            for(const auto& output : outputs)
            {
                i64 timestamp = get_file_timestamp(output);
                if(!timestamp)
                {
                    return 0;
                }
                ret = min(ret, timestamp);
            }
            return ret == I64_MAX ? 0 : ret;
        }

        static bool any_output_missing(MakeNode* node)
        {
            for(const auto& output : get_node_outputs(node))
            {
                if(!file_exists(output))
                {
                    return true;
                }
            }
            return false;
        }

        static i64 max_path_timestamp(const Vector<Path>& paths)
        {
            i64 ret = 0;
            for(const auto& path : paths)
            {
                ret = max(ret, get_file_timestamp(path));
            }
            return ret;
        }

        static RV validate_outputs(MakeNode* node)
        {
            if(!node->command)
            {
                return ok;
            }
            for(const auto& output : get_node_outputs(node))
            {
                if(!file_exists(output))
                {
                    return set_error(E_BAD_ARGUMENTS, "Build command for node %s did not produce output %s", node->path.encode().c_str(), output.encode().c_str());
                }
            }
            return ok;
        }

        static bool is_depfile_separator(c8 ch)
        {
            return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
        }

        static Vector<Path> parse_depfile_data(const String& data)
        {
            Vector<Path> ret;
            bool after_colon = false;
            String token;
            for(usize i = 0; i < data.size(); ++i)
            {
                c8 ch = data[i];
                if(ch == '\\' && i + 1 < data.size())
                {
                    c8 next = data[i + 1];
                    if(next == '\r' || next == '\n')
                    {
                        ++i;
                        if(next == '\r' && i + 1 < data.size() && data[i + 1] == '\n')
                        {
                            ++i;
                        }
                        continue;
                    }
                    token.push_back(next);
                    ++i;
                    continue;
                }
                if(!after_colon)
                {
                    if(ch == ':' && (i + 1 == data.size() || is_depfile_separator(data[i + 1])))
                    {
                        after_colon = true;
                        token.clear();
                    }
                    continue;
                }
                if(is_depfile_separator(ch))
                {
                    if(!token.empty())
                    {
                        ret.push_back(Path(token));
                        token.clear();
                    }
                    continue;
                }
                token.push_back(ch);
            }
            if(after_colon && !token.empty())
            {
                ret.push_back(Path(token));
            }
            return ret;
        }

        static R<Vector<Path>> load_depfile_inputs(Span<MakeNode* const> depfiles)
        {
            Vector<Path> ret;
            lutry
            {
                for(MakeNode* depfile : depfiles)
                {
                    auto file = open_file(depfile->path.encode().c_str(), FileOpenFlag::read | FileOpenFlag::user_buffering, FileCreationMode::open_existing);
                    if(!file.valid())
                    {
                        if(file.errcode() == E_NOT_FOUND)
                        {
                            continue;
                        }
                        luthrow(file.errcode());
                    }
                    Ref<IFile> f = move(file.get());
                    lulet(blob, load_file_data(f));
                    String data((const c8*)blob.data(), blob.size());
                    auto inputs = parse_depfile_data(data);
                    for(auto& input : inputs)
                    {
                        ret.push_back(move(input));
                    }
                }
            }
            lucatchret;
            return ret;
        }

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
            HashMap<Path, MakeNode*> nodes_by_path;
            lutry
            {
                for (MakeNode* target : targets)
                {
                    luexp(visit_node(target, visited, visiting, ordered));
                }
                for(MakeNode* node : ordered)
                {
                    if(node->path.empty())
                    {
                        return set_error(E_BAD_ARGUMENTS, "Make node path cannot be empty.");
                    }
                    auto iter = nodes_by_path.find(node->path);
                    if(iter != nodes_by_path.end() && iter->second != node)
                    {
                        return set_error(E_BAD_ARGUMENTS, "Duplicate make node path: %s", node->path.encode().c_str());
                    }
                    nodes_by_path.insert_or_assign(node->path, node);
                }
            }
            lucatchret;
            return ordered;
        }

        static RV visit_node(MakeNode* node, HashSet<MakeNode*>& visited, HashSet<MakeNode*>& visiting, Vector<MakeNode*>& ordered)
        {
            if(visiting.contains(node))
            {
                return set_error(E_BAD_ARGUMENTS, "Circular dependency detected at node: %s", node->path.encode().c_str());
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
            for(MakeNode* dep : node->order_only_dependencies)
            {
                auto r = visit_node(dep, visited, visiting, ordered);
                if(failed(r)) return r;
            }
            for(MakeNode* output : node->outputs)
            {
                auto r = visit_node(output, visited, visiting, ordered);
                if(failed(r)) return r;
            }
            for(MakeNode* depfile : node->depfiles)
            {
                auto r = visit_node(depfile, visited, visiting, ordered);
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
            bool m_succeeded = false;

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
                    if(r.errcode() == E_ERROR_OBJECT)
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
            else
            {
                ctx->m_succeeded = true;
            }
        }

        struct MakeNodeBuildInfo
        {
            MakeNode* node = nullptr;
            bool needs_build = false;
            bool scheduled = false;
            bool finished = false;
            usize remaining_deps = 0;
            Vector<usize> dependents;
            Vector<Path> cached_implicit_dependencies;
            u64 action_key = 0;
        };

        static void update_cache_record(BuildCache& cache, MakeNodeBuildInfo& info)
        {
            MakeNode* node = info.node;
            BuildCacheRecord record;
            record.command = node->action.command;
            record.action_key = info.action_key;
            record.outputs = get_node_outputs(node);
            auto dep_inputs = load_depfile_inputs(node->depfiles.cspan());
            if(dep_inputs.valid())
            {
                record.implicit_dependencies = move(dep_inputs.get());
            }
            else
            {
                record.implicit_dependencies = info.cached_implicit_dependencies;
            }
            record.action_key = compute_action_key(node);
            i64 timestamp = get_outputs_timestamp(node);
            if(!timestamp)
            {
                timestamp = get_utc_timestamp();
            }
            record.timestamp = timestamp;
            cache.set_record(node->path, record);
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

                HashMap<MakeNode*, usize> node_indices;
                HashSet<MakeNode*> generated_side_nodes;
                Vector<MakeNodeBuildInfo> build_infos;
                build_infos.reserve(ordered_nodes.size());
                for(usize i = 0; i < ordered_nodes.size(); ++i)
                {
                    for(MakeNode* output : ordered_nodes[i]->outputs)
                    {
                        generated_side_nodes.insert(output);
                    }
                    for(MakeNode* depfile : ordered_nodes[i]->depfiles)
                    {
                        generated_side_nodes.insert(depfile);
                    }
                    MakeNodeBuildInfo info;
                    info.node = ordered_nodes[i];
                    auto record = m_build_cache.get_record(info.node->path);
                    if(record.valid())
                    {
                        info.cached_implicit_dependencies = record.get().implicit_dependencies;
                    }
                    info.action_key = compute_action_key(info.node);
                    node_indices.insert_or_assign(info.node, i);
                    build_infos.push_back(move(info));
                }

                for(usize i = 0; i < build_infos.size(); ++i)
                {
                    MakeNode* node = build_infos[i].node;
                    i64 current_timestamp = get_node_timestamp(node);
                    i64 max_dep_timestamp = 0;
                    bool dependency_needs_build = false;

                    for (MakeNode* dependency : node->dependencies)
                    {
                        // If the dependency needs rebuild.
                        auto dep_iter = node_indices.find(dependency);
                        if(dep_iter != node_indices.end() && build_infos[dep_iter->second].needs_build)
                        {
                            dependency_needs_build = true;
                        }
                        max_dep_timestamp = max(max_dep_timestamp, get_node_timestamp(dependency));
                    }
                    max_dep_timestamp = max(max_dep_timestamp, max_path_timestamp(build_infos[i].cached_implicit_dependencies));

                    auto record = m_build_cache.get_record(node->path);
                    bool command_changed = !record.valid() || strcmp(record.get().command.c_str(), node->action.command.c_str());
                    bool action_key_changed = !record.valid() || record.get().action_key != build_infos[i].action_key;
                    bool missing_file = node_has_real_file(node) && any_output_missing(node);
                    bool should_build = false;
                    if(node->command)
                    {
                        should_build = node->force_rebuild || missing_file || command_changed || action_key_changed || max_dep_timestamp > current_timestamp || dependency_needs_build;
                    }
                    else
                    {
                        if(node->kind == MakeNodeKind::file && missing_file && !generated_side_nodes.contains(node))
                        {
                            return set_error(E_NOT_FOUND, "Input file node is missing and has no command: %s", node->path.encode().c_str());
                        }
                        should_build = node->kind == MakeNodeKind::phony ? dependency_needs_build : false;
                    }
                    if(should_build)
                    {
                        build_infos[i].needs_build = true;
                    }
                }

                usize num_commands_to_build = 0;
                for(auto& info : build_infos)
                {
                    if(info.needs_build && info.node->command)
                    {
                        ++num_commands_to_build;
                    }
                }
                if(num_commands_to_build == 0)
                {
                    log_info(m_log_handler, LOG_TAG, "Nothing to make.");
                    return ok;
                }

                for(usize i = 0; i < build_infos.size(); ++i)
                {
                    if(!build_infos[i].needs_build)
                    {
                        continue;
                    }
                    HashSet<MakeNode*> all_deps;
                    for(MakeNode* dep : build_infos[i].node->dependencies)
                    {
                        all_deps.insert(dep);
                    }
                    for(MakeNode* dep : build_infos[i].node->order_only_dependencies)
                    {
                        all_deps.insert(dep);
                    }
                    for(MakeNode* dep : all_deps)
                    {
                        auto iter = node_indices.find(dep);
                        if(iter != node_indices.end() && build_infos[iter->second].needs_build)
                        {
                            ++build_infos[i].remaining_deps;
                            build_infos[iter->second].dependents.push_back(i);
                        }
                    }
                }

                MakeNodeGlobalContext env;
                env.m_log_handler = &m_log_handler;
                env.m_num_jobs = num_commands_to_build;
                env.m_num_finished_jobs = 0;
                usize num_finished_nodes = 0;
                for(auto& info : build_infos)
                {
                    if(!info.needs_build)
                    {
                        ++num_finished_nodes;
                    }
                }
                while(num_finished_nodes < build_infos.size())
                {
                    Vector<usize> ready_indices;
                    for(usize i = 0; i < build_infos.size(); ++i)
                    {
                        auto& info = build_infos[i];
                        if(info.needs_build && !info.scheduled && info.remaining_deps == 0)
                        {
                            ready_indices.push_back(i);
                        }
                    }
                    if(ready_indices.empty())
                    {
                        return set_error(E_BAD_ARGUMENTS, "Internal MakeSystem scheduler deadlock.");
                    }

                    Vector<JobSystem::job_id_t> job_ids;
                    Vector<UniquePtr<MakeNodeContext>> job_ctxs;
                    for(usize index : ready_indices)
                    {
                        auto& info = build_infos[index];
                        info.scheduled = true;
                        if(!info.node->command)
                        {
                            update_cache_record(m_build_cache, info);
                            info.finished = true;
                            ++num_finished_nodes;
                            for(usize dependent : info.dependents)
                            {
                                --build_infos[dependent].remaining_deps;
                            }
                            continue;
                        }
                        UniquePtr<MakeNodeContext> ctx(memnew<MakeNodeContext>());
                        ctx->m_env = &env;
                        ctx->m_command = info.node->command;
                        ctx->m_display_info = info.node->display_info;
                        auto id = m_job_scheduler->submit_job(make_task_entry, ctx.get());
                        job_ids.push_back(id);
                        job_ctxs.push_back(move(ctx));
                    }
                    if(!job_ids.empty())
                    {
                        m_job_scheduler->wait_jobs(job_ids.cspan());
                    }
                    if(env.m_cancelled)
                    {
                        get_error() = env.m_error;
                        return E_ERROR_OBJECT;
                    }
                    usize job_ctx_index = 0;
                    for(usize index : ready_indices)
                    {
                        auto& info = build_infos[index];
                        if(!info.node->command)
                        {
                            continue;
                        }
                        if(!job_ctxs[job_ctx_index]->m_succeeded)
                        {
                            return set_error(E_BAD_ARGUMENTS, "Build command failed for node %s", info.node->path.encode().c_str());
                        }
                        ++job_ctx_index;
                        luexp(validate_outputs(info.node));
                        update_cache_record(m_build_cache, info);
                        info.finished = true;
                        ++num_finished_nodes;
                        for(usize dependent : info.dependents)
                        {
                            --build_infos[dependent].remaining_deps;
                        }
                    }
                }
                luexp(m_build_cache.save_to_file());
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
            if(node_has_real_file(node))
            {
                return get_outputs_timestamp(node);
            }
            return m_build_cache.get_timestamp(node->path);
        }

        LUNA_MAKE_SYSTEM_API R<Ref<IMakeSystem>> new_make_system(const Path& build_dir, u32 max_num_parallel_tasks)
        {
            lutry
            {
                auto ret = new_object<MakeSystem>();
                luexp(ret->init(build_dir, max_num_parallel_tasks));
                return Ref<IMakeSystem>(ret);
            }
            lucatchret;
            return E_ERROR_OBJECT;
        }
    }

    struct MakeSystemModule : public Module
    {
        virtual const c8* get_name() override { return "MakeSystem"; }
        virtual RV on_register() override
        {
            return add_dependency_module(this, module_job_system());
        }
        virtual RV on_init() override
        {
            Meta::register_MakeSystem_types();
            return ok;
        }
    };

    LUNA_MAKE_SYSTEM_API Module* module_make_system()
    {
        static MakeSystemModule m;
        return &m;
    }
}
