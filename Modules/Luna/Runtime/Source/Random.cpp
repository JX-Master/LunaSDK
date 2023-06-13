/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Random.cpp
* @author JXMaster
* @date 2019/12/26
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "Random.hpp"
#include "../Mutex.hpp"
#include <Luna/Runtime/Time.hpp>
namespace Luna
{
	std::mt19937 g_random_engine;
	Ref<IMutex> g_random_mutex;
	void random_init()
	{
		register_boxed_type<Random>();
		impl_interface_for_type<Random, IRandom>();
		g_random_engine.seed((unsigned int)get_ticks());
		g_random_mutex = new_mutex();
	}
	void random_close()
	{
		g_random_mutex = nullptr;
	}
	LUNA_RUNTIME_API Ref<IRandom> new_random(u32 initial_seed)
	{
		auto ret = new_object<Random>();
		ret->set_seed(initial_seed);
		return ret;
	}
	LUNA_RUNTIME_API u32 random_u32()
	{
		g_random_mutex->wait();
		u32 r = g_random_engine();
		g_random_mutex->unlock();
		return r;
	}
	LUNA_RUNTIME_API i32 random_i32()
	{
		g_random_mutex->wait();
		i32 r = (i32)g_random_engine();
		g_random_mutex->unlock();
		return r;
	}
	LUNA_RUNTIME_API u64 random_u64()
	{
		g_random_mutex->wait();
		u64 r = ((u64)g_random_engine()) + (((u64)g_random_engine()) << 32);
		g_random_mutex->unlock();
		return r;
	}
	LUNA_RUNTIME_API i64 random_i64()
	{
		g_random_mutex->wait();
		i64 r = ((i64)g_random_engine()) + (((i64)g_random_engine()) << 32);
		g_random_mutex->unlock();
		return r;
	}
	LUNA_RUNTIME_API f32 random_f32(f32 range_begin, f32 range_end)
	{
		g_random_mutex->wait();
		std::uniform_real_distribution<f32> dis(range_begin, range_end);
		f64 r = dis(g_random_engine);
		g_random_mutex->unlock();
		return r;
	}
	LUNA_RUNTIME_API f64 random_f64(f64 range_begin, f64 range_end)
	{
		g_random_mutex->wait();
		std::uniform_real_distribution<f64> dis(range_begin, range_end);
		f64 r = dis(g_random_engine);
		g_random_mutex->unlock();
		return r;
	}
	LUNA_RUNTIME_API Guid random_guid()
	{
		g_random_mutex->wait();
		Guid guid;
		guid.low = random_u64();
		guid.high = random_u64();
		g_random_mutex->unlock();
		return guid;
	}
}