/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file JobSystem.cpp
* @author JXMaster
* @date 2022/7/7
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_JOBSYSTEM_API LUNA_EXPORT
#include "../JobSystem.hpp"
#include <Luna/Runtime/RingDeque.hpp>
#include <Luna/Runtime/SpinLock.hpp>
#include <Luna/Runtime/Signal.hpp>
#include <Luna/Runtime/Random.hpp>
#include <Luna/Runtime/Module.hpp>

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
		inline void init_job_state_map()
		{
			g_next_job_id = 0;
			g_job_state_map_offset = 0;
		}
		inline void close_job_state_map()
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

		struct JobHeader
		{
			job_id_t m_id;
			job_func_t* m_func;
			JobHeader* m_parent;
			usize m_alignment;
			volatile u32 m_unfinished_jobs;

			bool is_completed() const
			{
				return m_unfinished_jobs == 0;
			}
			void* get_params() const
			{
				return (void*)((usize)this + sizeof(JobHeader));
			}
			static usize get_padding_size(usize alignment)
			{
				usize padding_count = sizeof(JobHeader) / alignment + ((sizeof(JobHeader) % alignment) ? 1 : 0);
				return padding_count * alignment;
			}
		};

		inline JobHeader* get_job_header(void* params)
		{
			return (JobHeader*)(((usize)params) - sizeof(JobHeader));
		}

		LUNA_JOBSYSTEM_API void* new_job(job_func_t* func, usize param_size, usize param_alignment, void* parent)
		{
			// Allocate extra padding space for storing job header.
			param_alignment = max(param_alignment, MAX_ALIGN);
			usize padding_size = JobHeader::get_padding_size(param_alignment);
			void* mem = memalloc(param_size + padding_size, param_alignment);
			void* params = (void*)((usize)mem + padding_size);
			JobHeader* job = get_job_header(params);
			new (job) JobHeader();
			job->m_id = INVALID_JOB_ID;
			job->m_func = func;
			job->m_parent = nullptr;
			job->m_alignment = param_alignment;
			job->m_unfinished_jobs = 1;
			if (parent)
			{
				job->m_parent = get_job_header(parent);
				atom_inc_u32(&(job->m_parent->m_unfinished_jobs));
			}
			return params;
		}

		struct WorkerThreadContext
		{
			SpinLock m_lock;
			RingDeque<JobHeader*> m_jobs;
			Ref<ISignal> m_wake_signal;
			bool m_thread_dead = false;
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
			for (u32 i = 0; i < processor_count - 1; ++i)
			{
				Ref<IThread> worker = new_thread(worker_thread_run, nullptr);
				g_worker_threads.push_back(worker);
			}
			// Consume job id 0, so any valid job will never have ID 0.
			job_id_t dummy = allocate_job_id();
			finish_job_id(dummy);
			return ok;
		}
		void job_system_close()
		{
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
			close_job_state_map();
		}
		static WorkerThreadContext* get_current_thread_worker_context()
		{
			WorkerThreadContext* ctx = (WorkerThreadContext*)tls_get(g_worker_thread_tls);
			if (!ctx)
			{
				// For working on user-created threads.
				ctx = memnew<WorkerThreadContext>();
				tls_set(g_worker_thread_tls, ctx);
				g_worker_thread_contexts_lock.lock();
				g_worker_thread_contexts.push_back(ctx);
				g_worker_thread_contexts_lock.unlock();
			}
			return ctx;
		}
		inline JobHeader* steal_job(WorkerThreadContext* current_ctx)
		{
			LockGuard guard(g_worker_thread_contexts_lock);
			if (g_worker_thread_contexts.empty()) return nullptr;
			u32 rand_index = random_u32() % (u32)g_worker_thread_contexts.size();
			usize i = 0;
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
					JobHeader* job = nullptr;
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
		static JobHeader* consume_job()
		{
			WorkerThreadContext* ctx = get_current_thread_worker_context();
			ctx->m_lock.lock();
			if (ctx->m_jobs.empty())
			{
				ctx->m_lock.unlock();
				// Steal jobs from other threads.
				JobHeader* job = steal_job(ctx);
				if (!job)
				{
					yield_current_thread();
					return nullptr;
				}
				return job;
			}
			else
			{
				JobHeader* job = ctx->m_jobs.back();
				ctx->m_jobs.pop_back();
				ctx->m_lock.unlock();
				return job;
			}
		}
		static void finish_job(JobHeader* job)
		{
			u32 unfinished_jobs = atom_dec_u32(&(job->m_unfinished_jobs));
			if (unfinished_jobs == 0)
			{
				if (job->m_parent)
				{
					finish_job(job->m_parent);
				}
				finish_job_id(job->m_id);
				usize alignment = job->m_alignment;
				usize padding_size = JobHeader::get_padding_size(alignment);
				void* raw_ptr = (void*)((usize)job->get_params() - padding_size);
				job->~JobHeader();
				memfree(raw_ptr, alignment);
			}
		}
		static void execute_job(JobHeader* job)
		{
			job->m_func(job->get_params());
			finish_job(job);
		}
		static void worker_thread_sleep()
		{
			WorkerThreadContext* ctx = get_current_thread_worker_context();
			if (!ctx->m_wake_signal) ctx->m_wake_signal = new_signal(false);
			// Add the current thread to sleep list.
			g_sleep_worker_threads_lock.lock();
			g_sleep_worker_threads.push_back(ctx);
			g_sleep_worker_threads_lock.unlock();
			ctx->m_wake_signal->wait();
		}
		static void worker_thread_run(void* params)
		{
			while (!g_job_system_exiting)
			{
				JobHeader* job = consume_job();
				if (job)
				{
					execute_job(job);
				}
				else
				{
					worker_thread_sleep();
				}
			}
		}
		LUNA_JOBSYSTEM_API job_id_t submit_job(void* params)
		{
			JobHeader* job = get_job_header(params);
			job_id_t id = allocate_job_id();
			job->m_id = id;
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
		LUNA_JOBSYSTEM_API job_id_t get_current_job_id(void* params)
		{
			JobHeader* job = get_job_header(params);
			return job->m_id;
		}
		LUNA_JOBSYSTEM_API void wait_job(job_id_t job)
		{
			while (!is_job_finished(job))
			{
				JobHeader* next_job = consume_job();
				if (next_job)
				{
					execute_job(next_job);
				}
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
