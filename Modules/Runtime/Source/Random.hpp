/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Random.hpp
* @author JXMaster
* @date 2019/3/24
*/
#pragma once
#include "../Random.hpp"
#include <random>

namespace Luna
{
	struct Random : IRandom
	{
		lustruct("Random", "{4f09c790-fa3c-4613-b511-2d0175e15582}");
		luiimpl();

		std::mt19937 m_engine;

		Random() {}

		void set_seed(u32 seed)
		{
			m_engine.seed(seed);
		}
		u32 gen_u32()
		{
			return m_engine();
		}
		i32 gen_i32()
		{
			return (i32)m_engine();
		}
		u64 gen_u64()
		{
			return ((u64)m_engine()) + (((u64)m_engine()) << 32);
		}
		i64 gen_i64()
		{
			return ((i64)m_engine()) + (((i64)m_engine()) << 32);
		}
		f32 gen_f32(f32 range_begin, f32 range_end)
		{
			return (f32)((((f64)gen_u64() / (f64)UINT64_MAX) - (f64)range_begin) * ((f64)range_end - (f64)range_begin));
		}
		f64 gen_f64(f64 range_begin, f64 range_end)
		{
			return (((f64)gen_u64() / (f64)UINT64_MAX) - range_begin) * (range_end - range_begin);
		}
		Guid gen_guid()
		{
			Guid guid;
			guid.low = gen_u64();
			guid.high = gen_u64();
			return guid;
		}
	};

	void random_init();
	void random_close();
}